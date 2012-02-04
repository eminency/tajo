/**
 * 
 */
package nta.engine.planner.logical;

import nta.catalog.Schema;
import nta.engine.exec.eval.EvalNode;
import nta.engine.parser.QueryBlock.FromTable;

/**
 * @author Hyunsik Choi
 *
 */
public class ScanNode extends LogicalNode {
  private FromTable table;
  private EvalNode qual;
  private Schema targetList;
  
	public ScanNode(FromTable table) {
		super(ExprType.SCAN);
		this.table = table;
	}
	
	public String getTableId() {
	  return table.getTableId();
	}
	
	public boolean hasAlias() {
	  return table.hasAlias();
	}
	
	public String getAlias() {
	  return table.getAlias();
	}
	
	public boolean hasQual() {
	  return qual != null;
	}
	
	public EvalNode getQual() {
	  return this.qual;
	}
	
	public void setQual(EvalNode evalTree) {
	  this.qual = evalTree;
	}
	
	public boolean hasTargetList() {
	  return this.targetList != null;
	}
	
	public void setTargetList(Schema targets) {
	  this.targetList = targets;
	}
	
	public Schema getTargetList() {
	  return this.targetList;
	}
	
	public String toString() {
	  StringBuilder sb = new StringBuilder();	  
	  sb.append("\"Scan\" : {\"table\":\"")
	  .append(table.getTableId()).append("\"");
	  if (hasAlias()) {
	    sb.append(",\"alias\": \"").append(table.getAlias());
	  }
	  
	  if (hasQual()) {
	    sb.append(", \"qual\": \"").append(this.qual).append("\"");
	  }
	  
	  if (hasTargetList()) {
	    sb.append(", \"target list\": ").append(this.targetList);
	  }
	  
	  sb.append(",");
	  sb.append("\n  \"out schema\": ").append(getOutputSchema());
	  sb.append("\n  \"in schema\": ").append(getInputSchema());    	  
	  return sb.toString();
	}
	
	public Object clone() {
	  ScanNode scanNode = new ScanNode(this.table);
	  if(hasQual()) {
	    scanNode.qual = this.qual;
	  }
	  
	  return scanNode;
	}
}
