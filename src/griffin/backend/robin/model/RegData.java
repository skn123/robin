package backend.robin.model;

public class RegData {

	public String name;
	public SimpleType type;
	public String proto;
	public String symbol;

	public RegData(String name, SimpleType type, String proto, String symbol)
	{
		this.name = name;
		this.type = type;
		this.proto = proto;
		this.symbol = symbol;
	}
	
}
