package nta.engine.planner;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

import java.util.Stack;

import nta.catalog.CatalogService;
import nta.catalog.FunctionDesc;
import nta.catalog.Options;
import nta.catalog.Schema;
import nta.catalog.TCatUtil;
import nta.catalog.TableDesc;
import nta.catalog.TableDescImpl;
import nta.catalog.TableMeta;
import nta.catalog.proto.CatalogProtos.DataType;
import nta.catalog.proto.CatalogProtos.FunctionType;
import nta.catalog.proto.CatalogProtos.IndexMethod;
import nta.catalog.proto.CatalogProtos.StoreType;
import nta.engine.NtaTestingUtility;
import nta.engine.QueryContext;
import nta.engine.exec.eval.EvalNode;
import nta.engine.function.SumInt;
import nta.engine.json.GsonCreator;
import nta.engine.parser.ParseTree;
import nta.engine.parser.QueryAnalyzer;
import nta.engine.planner.logical.CreateIndexNode;
import nta.engine.planner.logical.CreateTableNode;
import nta.engine.planner.logical.ExprType;
import nta.engine.planner.logical.GroupbyNode;
import nta.engine.planner.logical.JoinNode;
import nta.engine.planner.logical.LogicalNode;
import nta.engine.planner.logical.LogicalNodeVisitor;
import nta.engine.planner.logical.LogicalRootNode;
import nta.engine.planner.logical.ProjectionNode;
import nta.engine.planner.logical.ScanNode;
import nta.engine.planner.logical.SelectionNode;
import nta.engine.planner.logical.SortNode;

import org.apache.hadoop.fs.Path;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

import com.google.gson.Gson;

/**
 * @author Hyunsik Choi
 */
public class TestLogicalPlanner {
  private static NtaTestingUtility util;
  private static CatalogService catalog;
  private static QueryContext.Factory factory;
  private static QueryAnalyzer analyzer;

  @BeforeClass
  public static void setUp() throws Exception {
    util = new NtaTestingUtility();
    util.startMiniZKCluster();
    util.startCatalogCluster();
    catalog = util.getMiniCatalogCluster().getCatalog();
    
    Schema schema = new Schema();
    schema.addColumn("name", DataType.STRING);
    schema.addColumn("empId", DataType.INT);
    schema.addColumn("deptName", DataType.STRING);

    Schema schema2 = new Schema();
    schema2.addColumn("deptName", DataType.STRING);
    schema2.addColumn("manager", DataType.STRING);

    Schema schema3 = new Schema();
    schema3.addColumn("deptName", DataType.STRING);
    schema3.addColumn("score", DataType.INT);

    TableMeta meta = TCatUtil.newTableMeta(schema, StoreType.CSV);
    TableDesc people = new TableDescImpl("employee", meta, 
        new Path("file:///"));
    catalog.addTable(people);

    TableDesc student = new TableDescImpl("dept", schema2, StoreType.CSV,
        new Options(),
        new Path("file:///"));
    catalog.addTable(student);

    TableDesc score = new TableDescImpl("score", schema3, StoreType.CSV,
        new Options(),
        new Path("file:///"));
    catalog.addTable(score);

    FunctionDesc funcDesc = new FunctionDesc("sumtest", SumInt.class,
        FunctionType.GENERAL, DataType.INT, new DataType[] { DataType.INT });

    catalog.registerFunction(funcDesc);
    analyzer = new QueryAnalyzer(catalog);
    factory = new QueryContext.Factory(catalog);
  }

  @AfterClass
  public static void tearDown() throws Exception {
    util.shutdownCatalogCluster();
    util.shutdownMiniZKCluster();
  }

  static String[] QUERIES = {
      "select name, empId, deptName from employee where empId > 500", // 0
      "select name, empId, e.deptName, manager from employee as e, dept as dp", // 1
      "select name, empId, e.deptName, manager, score from employee as e, dept, score", // 2
      "select p.deptName, sumtest(score) from dept as p, score group by p.deptName having sumtest(score) > 30", // 3
      "select p.deptName, score from dept as p, score order by score asc", // 4
      "select name from employee where empId = 100", // 5
      "select name, score from employee, score", // 6
      "select p.deptName, sumtest(score) from dept as p, score group by p.deptName", // 7
      "store1 := select p.deptName, sumtest(score) from dept as p, score group by p.deptName", // 8
      "select deptName, sumtest(score) from score group by deptName having sumtest(score) > 30", // 9
      "select 7 + 8", // 10
      "create index idx_employee on employee using bitmap (name null first, empId desc) with (fillfactor = 70)" // 11
  };

  @Test
  public final void testSingleRelation() throws CloneNotSupportedException {
    QueryContext ctx = factory.create();
    ParseTree block = (ParseTree) analyzer.parse(ctx, QUERIES[0]);
    LogicalNode plan = LogicalPlanner.createPlan(ctx, block);
    assertEquals(ExprType.ROOT, plan.getType());
    TestLogicalNode.testCloneLogicalNode(plan);
    LogicalRootNode root = (LogicalRootNode) plan;

    assertEquals(ExprType.PROJECTION, root.getSubNode().getType());
    ProjectionNode projNode = (ProjectionNode) root.getSubNode();

    assertEquals(ExprType.SELECTION, projNode.getSubNode().getType());
    SelectionNode selNode = (SelectionNode) projNode.getSubNode();

    assertEquals(ExprType.SCAN, selNode.getSubNode().getType());
    ScanNode scanNode = (ScanNode) selNode.getSubNode();
    assertEquals("employee", scanNode.getTableId());
  }

  @Test
  public final void testImplicityJoinPlan() throws CloneNotSupportedException {
    // two relations
    QueryContext ctx = factory.create();
    ParseTree block = (ParseTree) analyzer.parse(ctx, QUERIES[1]);
    LogicalNode plan = LogicalPlanner.createPlan(ctx, block);

    assertEquals(ExprType.ROOT, plan.getType());
    LogicalRootNode root = (LogicalRootNode) plan;
    TestLogicalNode.testCloneLogicalNode(root);

    assertEquals(ExprType.PROJECTION, root.getSubNode().getType());
    ProjectionNode projNode = (ProjectionNode) root.getSubNode();

    assertEquals(ExprType.JOIN, projNode.getSubNode().getType());
    JoinNode joinNode = (JoinNode) projNode.getSubNode();

    assertEquals(ExprType.SCAN, joinNode.getOuterNode().getType());
    ScanNode leftNode = (ScanNode) joinNode.getOuterNode();
    assertEquals("employee", leftNode.getTableId());
    assertEquals(ExprType.SCAN, joinNode.getInnerNode().getType());
    ScanNode rightNode = (ScanNode) joinNode.getInnerNode();
    assertEquals("dept", rightNode.getTableId());

    // three relations
    ctx = factory.create();
    block = (ParseTree) analyzer.parse(ctx, QUERIES[2]);
    plan = LogicalPlanner.createPlan(ctx, block);
    System.out.println(plan);
    TestLogicalNode.testCloneLogicalNode(plan);

    assertEquals(ExprType.ROOT, plan.getType());
    root = (LogicalRootNode) plan;

    assertEquals(ExprType.PROJECTION, root.getSubNode().getType());
    projNode = (ProjectionNode) root.getSubNode();

    assertEquals(ExprType.JOIN, projNode.getSubNode().getType());
    joinNode = (JoinNode) projNode.getSubNode();

    assertEquals(ExprType.SCAN, joinNode.getOuterNode().getType());
    ScanNode scan1 = (ScanNode) joinNode.getOuterNode();
    assertEquals("employee", scan1.getTableId());
    
    JoinNode rightNode2 = (JoinNode) joinNode.getInnerNode();
    assertEquals(ExprType.JOIN, rightNode2.getType());
        
    assertEquals(ExprType.SCAN, rightNode2.getOuterNode().getType());
    ScanNode leftScan = (ScanNode) rightNode2.getOuterNode();
    assertEquals("dept", leftScan.getTableId());

    assertEquals(ExprType.SCAN, rightNode2.getInnerNode().getType());
    ScanNode rightScan = (ScanNode) rightNode2.getInnerNode();
    assertEquals("score", rightScan.getTableId());
  }
  
  String [] JOINS = { 
      "select name, dept.deptName, score from employee natural join dept natural join score", // 0
      "select name, dept.deptName, score from employee inner join dept inner join score on dept.deptName = score.deptName", // 1
      "select name, dept.deptName, score from employee left outer join dept right outer join score on dept.deptName = score.deptName" // 2
  };
  
  @Test
  public final void testNaturalJoinPlan() {
    // two relations
    QueryContext ctx = factory.create();
    ParseTree block = (ParseTree) analyzer.parse(ctx, JOINS[0]);
    LogicalNode plan = LogicalPlanner.createPlan(ctx, block);
    assertEquals(ExprType.ROOT, plan.getType());
    LogicalRootNode root = (LogicalRootNode) plan;    
    assertEquals(ExprType.PROJECTION, root.getSubNode().getType());
    ProjectionNode proj = (ProjectionNode) root.getSubNode();
    assertEquals(ExprType.JOIN, proj.getSubNode().getType());
    JoinNode join = (JoinNode) proj.getSubNode();
    assertEquals(JoinType.NATURAL, join.getJoinType());
    assertEquals(ExprType.SCAN, join.getOuterNode().getType());
    assertTrue(join.hasJoinQual());
    System.out.println(join.getJoinQual());
    ScanNode scan = (ScanNode) join.getOuterNode();
    assertEquals("employee", scan.getTableId());
    
    assertEquals(ExprType.JOIN, join.getInnerNode().getType());
    join = (JoinNode) join.getInnerNode();
    assertEquals(JoinType.NATURAL, join.getJoinType());
    assertEquals(ExprType.SCAN, join.getOuterNode().getType());
    ScanNode outer = (ScanNode) join.getOuterNode();
    assertEquals("dept", outer.getTableId());
    assertEquals(ExprType.SCAN, join.getInnerNode().getType());
    ScanNode inner = (ScanNode) join.getInnerNode();
    assertEquals("score", inner.getTableId());
  }
  
  @Test
  public final void testInnerJoinPlan() {
    // two relations
    QueryContext ctx = factory.create();
    ParseTree block = (ParseTree) analyzer.parse(ctx, JOINS[1]);
    LogicalNode plan = LogicalPlanner.createPlan(ctx, block);
    assertEquals(ExprType.ROOT, plan.getType());
    LogicalRootNode root = (LogicalRootNode) plan;    
    assertEquals(ExprType.PROJECTION, root.getSubNode().getType());
    ProjectionNode proj = (ProjectionNode) root.getSubNode();
    assertEquals(ExprType.JOIN, proj.getSubNode().getType());
    JoinNode join = (JoinNode) proj.getSubNode();
    assertEquals(JoinType.INNER, join.getJoinType());
    assertEquals(ExprType.SCAN, join.getOuterNode().getType());
    ScanNode scan = (ScanNode) join.getOuterNode();
    assertEquals("employee", scan.getTableId());    
    
    assertEquals(ExprType.JOIN, join.getInnerNode().getType());
    join = (JoinNode) join.getInnerNode();
    assertEquals(JoinType.INNER, join.getJoinType());
    assertEquals(ExprType.SCAN, join.getOuterNode().getType());
    ScanNode outer = (ScanNode) join.getOuterNode();
    assertEquals("dept", outer.getTableId());
    assertEquals(ExprType.SCAN, join.getInnerNode().getType());
    ScanNode inner = (ScanNode) join.getInnerNode();
    assertEquals("score", inner.getTableId());
    assertTrue(join.hasJoinQual());
    assertEquals(EvalNode.Type.EQUAL, join.getJoinQual().getType());
  }
  
  @Test
  public final void testOuterJoinPlan() {
    // two relations
    QueryContext ctx = factory.create();
    ParseTree block = (ParseTree) analyzer.parse(ctx, JOINS[2]);
    LogicalNode plan = LogicalPlanner.createPlan(ctx, block);
    assertEquals(ExprType.ROOT, plan.getType());
    LogicalRootNode root = (LogicalRootNode) plan;    
    assertEquals(ExprType.PROJECTION, root.getSubNode().getType());
    ProjectionNode proj = (ProjectionNode) root.getSubNode();
    assertEquals(ExprType.JOIN, proj.getSubNode().getType());
    JoinNode join = (JoinNode) proj.getSubNode();
    assertEquals(JoinType.LEFT_OUTER, join.getJoinType());
    assertEquals(ExprType.SCAN, join.getOuterNode().getType());
    ScanNode scan = (ScanNode) join.getOuterNode();
    assertEquals("employee", scan.getTableId());
    
    assertEquals(ExprType.JOIN, join.getInnerNode().getType());
    join = (JoinNode) join.getInnerNode();
    assertEquals(JoinType.RIGHT_OUTER, join.getJoinType());
    assertEquals(ExprType.SCAN, join.getOuterNode().getType());
    ScanNode outer = (ScanNode) join.getOuterNode();
    assertEquals("dept", outer.getTableId());
    assertEquals(ExprType.SCAN, join.getInnerNode().getType());
    ScanNode inner = (ScanNode) join.getInnerNode();
    assertEquals("score", inner.getTableId());
    assertTrue(join.hasJoinQual());
    assertEquals(EvalNode.Type.EQUAL, join.getJoinQual().getType());
  }

  @Test
  public final void testGroupby() throws CloneNotSupportedException {
    // without 'having clause'
    QueryContext ctx = factory.create();
    ParseTree block = (ParseTree) analyzer.parse(ctx, QUERIES[7]);
    LogicalNode plan = LogicalPlanner.createPlan(ctx, block);

    assertEquals(ExprType.ROOT, plan.getType());
    LogicalRootNode root = (LogicalRootNode) plan;
    testQuery7(root.getSubNode());
    
    // with having clause
    ctx = factory.create();
    block = (ParseTree) analyzer.parse(ctx, QUERIES[3]);
    plan = LogicalPlanner.createPlan(ctx, block);
    TestLogicalNode.testCloneLogicalNode(plan);

    assertEquals(ExprType.ROOT, plan.getType());
    root = (LogicalRootNode) plan;

    assertEquals(ExprType.GROUP_BY, root.getSubNode().getType());
    GroupbyNode groupByNode = (GroupbyNode) root.getSubNode();

    assertEquals(ExprType.JOIN, groupByNode.getSubNode().getType());
    JoinNode joinNode = (JoinNode) groupByNode.getSubNode();

    assertEquals(ExprType.SCAN, joinNode.getOuterNode().getType());
    ScanNode leftNode = (ScanNode) joinNode.getOuterNode();
    assertEquals("dept", leftNode.getTableId());
    assertEquals(ExprType.SCAN, joinNode.getInnerNode().getType());
    ScanNode rightNode = (ScanNode) joinNode.getInnerNode();
    assertEquals("score", rightNode.getTableId());
    
    System.out.println(plan);
    System.out.println("-------------------");
    LogicalOptimizer.optimize(ctx, plan);
    System.out.println(plan);
  }
  
  static void testQuery7(LogicalNode plan) {
    assertEquals(ExprType.GROUP_BY, plan.getType());
    GroupbyNode groupByNode = (GroupbyNode) plan;

    assertEquals(ExprType.JOIN, groupByNode.getSubNode().getType());
    JoinNode joinNode = (JoinNode) groupByNode.getSubNode();

    assertEquals(ExprType.SCAN, joinNode.getOuterNode().getType());
    ScanNode leftNode = (ScanNode) joinNode.getOuterNode();
    assertEquals("dept", leftNode.getTableId());
    assertEquals(ExprType.SCAN, joinNode.getInnerNode().getType());
    ScanNode rightNode = (ScanNode) joinNode.getInnerNode();
    assertEquals("score", rightNode.getTableId());
  }
  
  @Test
  public final void testStoreTable() throws CloneNotSupportedException {
    QueryContext ctx = factory.create();
    ParseTree tree = (ParseTree) analyzer.parse(ctx, QUERIES[8]);
    LogicalNode plan = LogicalPlanner.createPlan(ctx, tree);
    TestLogicalNode.testCloneLogicalNode(plan);
    
    assertEquals(ExprType.ROOT, plan.getType());
    LogicalRootNode root = (LogicalRootNode) plan;
    
    assertEquals(ExprType.STORE, root.getSubNode().getType());
    CreateTableNode storeNode = (CreateTableNode) root.getSubNode();
    testQuery7(storeNode.getSubNode());
    LogicalOptimizer.optimize(ctx, plan);
  }

  @Test
  public final void testOrderBy() throws CloneNotSupportedException {
    QueryContext ctx = factory.create();
    ParseTree block = (ParseTree) analyzer.parse(ctx, QUERIES[4]);
    LogicalNode plan = LogicalPlanner.createPlan(ctx, block);
    TestLogicalNode.testCloneLogicalNode(plan);

    assertEquals(ExprType.ROOT, plan.getType());
    LogicalRootNode root = (LogicalRootNode) plan;

    assertEquals(ExprType.PROJECTION, root.getSubNode().getType());
    ProjectionNode projNode = (ProjectionNode) root.getSubNode();

    assertEquals(ExprType.SORT, projNode.getSubNode().getType());
    SortNode sortNode = (SortNode) projNode.getSubNode();

    assertEquals(ExprType.JOIN, sortNode.getSubNode().getType());
    JoinNode joinNode = (JoinNode) sortNode.getSubNode();

    assertEquals(ExprType.SCAN, joinNode.getOuterNode().getType());
    ScanNode leftNode = (ScanNode) joinNode.getOuterNode();
    assertEquals("dept", leftNode.getTableId());
    assertEquals(ExprType.SCAN, joinNode.getInnerNode().getType());
    ScanNode rightNode = (ScanNode) joinNode.getInnerNode();
    assertEquals("score", rightNode.getTableId());
  }

  @Test
  public final void testSPJPush() throws CloneNotSupportedException {
    QueryContext ctx = factory.create();
    ParseTree block = (ParseTree) analyzer.parse(ctx, QUERIES[5]);
    LogicalNode plan = LogicalPlanner.createPlan(ctx, block);
    TestLogicalNode.testCloneLogicalNode(plan);
    
    assertEquals(ExprType.ROOT, plan.getType());
    LogicalRootNode root = (LogicalRootNode) plan;
    assertEquals(ExprType.PROJECTION, root.getSubNode().getType());
    ProjectionNode projNode = (ProjectionNode) root.getSubNode();
    assertEquals(ExprType.SELECTION, projNode.getSubNode().getType());
    SelectionNode selNode = (SelectionNode) projNode.getSubNode();    
    assertEquals(ExprType.SCAN, selNode.getSubNode().getType());
    ScanNode scanNode = (ScanNode) selNode.getSubNode();
    
    LogicalNode optimized = LogicalOptimizer.optimize(ctx, plan);
    assertEquals(ExprType.ROOT, optimized.getType());
    root = (LogicalRootNode) optimized;
    
    assertEquals(ExprType.SCAN, root.getSubNode().getType());
    scanNode = (ScanNode) root.getSubNode();
    assertEquals("employee", scanNode.getTableId());
  }
  
  @Test
  public final void testSPJ() throws CloneNotSupportedException {
    QueryContext ctx = factory.create();
    ParseTree block = (ParseTree) analyzer.parse(ctx, QUERIES[6]);
    LogicalNode plan = LogicalPlanner.createPlan(ctx, block);
    TestLogicalNode.testCloneLogicalNode(plan);
  }
  
  @Test
  public final void testJson() {
    QueryContext ctx = factory.create();
	  ParseTree block = (ParseTree) analyzer.parse(ctx, QUERIES[9]);
	  LogicalNode plan = LogicalPlanner.createPlan(ctx, block);
	  LogicalOptimizer.optimize(ctx, plan);
	    
	  String json = plan.toJSON();
	  System.out.println(json);
	  Gson gson = GsonCreator.getInstance();
	  LogicalNode fromJson = gson.fromJson(json, LogicalNode.class);
	  System.out.println(fromJson.toJSON());
	  assertEquals(ExprType.ROOT, fromJson.getType());
	  LogicalNode groupby = ((LogicalRootNode)fromJson).getSubNode();
	  assertEquals(ExprType.GROUP_BY, groupby.getType());
	  LogicalNode scan = ((GroupbyNode)groupby).getSubNode();
	  assertEquals(ExprType.SCAN, scan.getType());
  }
  
  @Test
  public final void testVisitor() {
    // two relations
    QueryContext ctx = factory.create();
    ParseTree block = (ParseTree) analyzer.parse(ctx, QUERIES[1]);
    LogicalNode plan = LogicalPlanner.createPlan(ctx, block);
    
    TestVisitor vis = new TestVisitor();
    plan.postOrder(vis);
    
    assertEquals(ExprType.ROOT, vis.stack.pop().getType());
    assertEquals(ExprType.PROJECTION, vis.stack.pop().getType());
    assertEquals(ExprType.JOIN, vis.stack.pop().getType());
    assertEquals(ExprType.SCAN, vis.stack.pop().getType());
    assertEquals(ExprType.SCAN, vis.stack.pop().getType());
  }
  
  private static class TestVisitor implements LogicalNodeVisitor {
    Stack<LogicalNode> stack = new Stack<LogicalNode>();
    @Override
    public void visit(LogicalNode node) {
      stack.push(node);
    }
  }
  
  @Test
  public final void testExprNode() {
    QueryContext ctx = factory.create();
    ParseTree block = (ParseTree) analyzer.parse(ctx, QUERIES[10]);
    LogicalNode plan = LogicalPlanner.createPlan(ctx, block);
    LogicalOptimizer.optimize(ctx, plan);
    assertEquals(ExprType.ROOT, plan.getType());
    LogicalRootNode root = (LogicalRootNode) plan;
    assertEquals(ExprType.EXPRS, root.getSubNode().getType());
  }
  
  @Test
  public final void testCreateIndex() {
    QueryContext ctx = factory.create();
    ParseTree block = (ParseTree) analyzer.parse(ctx, QUERIES[11]);
    LogicalNode plan = LogicalPlanner.createPlan(ctx, block);
    LogicalOptimizer.optimize(ctx, plan);
    LogicalRootNode root = (LogicalRootNode) plan;
    
    assertEquals(ExprType.CREATE_INDEX, root.getSubNode().getType());
    CreateIndexNode indexNode = (CreateIndexNode) root.getSubNode();
    assertEquals("idx_employee", indexNode.getIndexName());
    assertEquals("employee", indexNode.getTableName());
    assertEquals(false, indexNode.isUnique());
    assertEquals(2, indexNode.getSortSpecs().length);
    assertEquals("name", indexNode.getSortSpecs()[0].getSortKey().getColumnName());
    assertEquals(DataType.STRING, indexNode.getSortSpecs()[0].getSortKey().getDataType());
    assertEquals(true, indexNode.getSortSpecs()[0].isNullFirst());
    assertEquals("empid", indexNode.getSortSpecs()[1].getSortKey().getColumnName());
    assertEquals(DataType.INT, indexNode.getSortSpecs()[1].getSortKey().getDataType());
    assertEquals(false, indexNode.getSortSpecs()[1].isAscending());
    assertEquals(false, indexNode.getSortSpecs()[1].isNullFirst());
    assertEquals(IndexMethod.BITMAP, indexNode.getMethod());
  }
}
