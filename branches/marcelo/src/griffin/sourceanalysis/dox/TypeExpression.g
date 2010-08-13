header {
package sourceanalysis.dox;
}

options {
	language = "Java";
}

/**
 * ANTLR Grammar for parsing C++ type-expressions
 */
 
class TypeExpressionLexer extends Lexer;
options {
    k = 3;
}
{
    static void silenceWarnings() {
        /* these statements thwart warnings about unused imports */
        ANTLRException k; CharScanner l; CommonToken t;
        MismatchedCharException mte; SemanticException se;
    }
}

 WS  :   (' '
    |   '\t'
    |   '\n'
    |   '\r')
        { $setType(antlr.Token.SKIP); }
    ;
    
protected
KEY : "virtual" | "static" | "mutable" | "MUTABLE" | "register" | "DLLTAG"
	 | "inline" | "friend" | "typename" | "struct" | "class" | "enum" | "template";

OPEN_PAREN: '(' ;
CLOSE_PAREN: ')' ;

OPEN_BRACKET: '[' ;
CLOSE_BRACKET: ']' ;

OPEN_ANGLE: '<' ;
CLOSE_ANGLE: '>' ;

STAR: '*' ;
AMPERSAND: '&' ;
COMMA: ',' ;

ID:	(KEY) => KEY { $setType(antlr.Token.SKIP); }
	 |  ('a'..'z' | 'A'..'Z' | '_') ('a'..'z' | 'A'..'Z' | '0'..'9' | '_')*
	 | '@' ('0'..'9')+
	;
	
INT: ( '-' | '+' )? ( '0'..'9' )+ ( 'l' | 'L' )?
	;

QUAD: ':' ':' ;

ELLIPSIS: '.' '.' '.';

INITIALIZER: '=' ( ( '0'..'9' )+ | ID ) { $setType(antlr.Token.SKIP); };


{
import sourceanalysis.*;
import java.util.List;
import java.util.LinkedList;
import java.util.Iterator;

import javax.swing.tree.DefaultMutableTreeNode;

}
class TypeExpressionParser extends Parser;
options {
	buildAST = false;
	k = 1;
}
{
	public void assignYellowPages(EntityNameResolving ypentries)
	{
		m_yellow_pages = ypentries;
	}

	public Entity peek(String name)
	{
		if (m_yellow_pages != null) {
			return m_yellow_pages.resolve(name);
		}
		Entity e = new Aggregate();
		e.setName(name);
		return e;
	}
	
	public Entity primitive(String name)
	{
		if (m_yellow_pages != null) {
			return m_yellow_pages.resolvePrimitive(name);
		}
		Entity e = new Primitive();
		e.setName(name);
		return e;
	}
	
	private Type.TypeNode array(Type.TypeNode base, int dimension)
	{
		if (base.getKind() == Type.TypeNode.NODE_X_COMPOSITION) {
			Type.TypeNode fnode = new Type.TypeNode(Type.TypeNode.NODE_X_COMPOSITION);
			Type.TypeNode sub = (Type.TypeNode)base.getChildAt(1);
			fnode.add((Type.TypeNode)base.getChildAt(0));
			fnode.add(array(sub, dimension));
			return fnode;
		}
		else {
			Type.TypeNode arrnode = new Type.TypeNode(Type.TypeNode.NODE_ARRAY);
			arrnode.add(blank());
			arrnode.add(new DefaultMutableTreeNode(new Integer(dimension)));
			Type.TypeNode compnode = new Type.TypeNode(Type.TypeNode.NODE_X_COMPOSITION);
			compnode.add(arrnode);
			compnode.add(base);
			return compnode;
		}
	}
	
	private Type.TypeNode func(Type.TypeNode base)
	{
		if (base.getKind() == Type.TypeNode.NODE_X_COMPOSITION) {
			Type.TypeNode fnode = new Type.TypeNode(Type.TypeNode.NODE_X_COMPOSITION);
			Type.TypeNode sub = (Type.TypeNode)base.getChildAt(1);
			fnode.add((Type.TypeNode)base.getChildAt(0));
			fnode.add(func(sub));
			return fnode;
		}
		else {
			Type.TypeNode fnode = new Type.TypeNode(Type.TypeNode.NODE_FUNCTION);
			fnode.add(base);
			return fnode;
		}
	}
	
	private Type.TypeNode ptr(Type.TypeNode base, Pointer pointer)
	{
		if (base.getKind() == Type.TypeNode.NODE_X_COMPOSITION) {
			Type.TypeNode fnode = new Type.TypeNode(Type.TypeNode.NODE_X_COMPOSITION);
			Type.TypeNode sub = (Type.TypeNode)base.getChildAt(1);
			fnode.add((Type.TypeNode)base.getChildAt(0));
			fnode.add(ptr(sub, pointer));
			return fnode;
		}
		else {
			Type.TypeNode pnode = new Type.TypeNode(pointer.nodeType);
			pnode.add(base);
			pnode.setCV(pointer.cv);
			return pnode;
		}
	}
	
	private Type.TypeNode ellipsis()
	{
		return new Type.TypeNode(Type.TypeNode.NODE_ELLIPSIS);
	}
	
	public void reportError(RecognitionException ex)
	{
		if (m_error_msg == null)
			m_error_msg = new StringBuffer("" + ex);
		else
			m_error_msg.append(" ; " + ex);
	}
	
	public boolean errorOccurred()
	{
		return (m_error_msg != null);
	}
	
	public String getErrorMessages()
	{
		return m_error_msg.toString();
	}
	
	class Pointer
	{
		public int nodeType;
		public int cv;
	}

	private static Type.TypeNode errorType()
	{
		if (m_error_entity == null) {
			m_error_entity = new Aggregate();
			m_error_entity.setName("<error>");
		}
		return new Type.TypeNode(m_error_entity);
	}
	
	private EntityNameResolving m_yellow_pages;
	private static Entity m_error_entity = null;
	
	private static Type.TypeNode m_err = errorType();
	private static Type.TypeNode blank() {
		return new Type.TypeNode(Type.TypeNode.NODE_X_BLANK);
	}
	
    static void silenceWarnings() {
        /* these statements thwart warnings about unused imports */
        ANTLRException k; LLkParser p;
	    MismatchedTokenException mte; SemanticException se; 
	    TokenStreamIOException tsioe;
	}
	
	private StringBuffer m_error_msg;
}

typeexpr returns [ sourceanalysis.Type.TypeNode node=m_err ]
	{ Type.TypeNode base = m_err; }
	:
	base=basename node=declarator[base]  (EOF)?
	;
	
basename returns [ sourceanalysis.Type.TypeNode node=m_err ]
	{ String namestring; int cvflags = Specifiers.CVQualifiers.NONE; int flag=0; }
	:
	(flag=cv { cvflags |= flag; } )*
	(
		(template_id)=>node=template_id
	  |	namestring=nested_name { node = new Type.TypeNode(peek(namestring)); }
	  |	node=basic_type
	)
	(
	 (QUAD)=>(QUAD namestring=nested_name { node = new Type.TypeNode(peek(node.formatCpp() + "::" + namestring)); })
	 |
	)
    (flag=cv { cvflags |= flag; })*
	{ if (node == m_err) node = errorType(); node.setCV(cvflags); }
	;

nonempty_direct_declarator[Type.TypeNode base] returns [Type.TypeNode decl=base]
	{ Type.TypeNode mid=m_err; Pointer pointer; }
	:
	(ptr_operator) => pointer=ptr_operator { mid = ptr(base, pointer); }
		decl=direct_declarator[mid]
  |	(OPEN_PAREN) => OPEN_PAREN mid=declarator[blank()] CLOSE_PAREN
  		{ decl = new Type.TypeNode(Type.TypeNode.NODE_X_COMPOSITION);
  		  decl.add(mid); decl.add(base); }
  |	ID
 	;
 	
direct_declarator[Type.TypeNode base] returns [Type.TypeNode decl=base]:
	((nonempty_direct_declarator[base]) 
	   => decl=nonempty_direct_declarator[base])?
	;

declarator[Type.TypeNode base] returns [Type.TypeNode decl=m_err]
	{ int dimension = 0; }
	:
	decl=direct_declarator[base]
	(
		OPEN_PAREN parameter_list CLOSE_PAREN
			{ decl = func(decl); }
	 |	{ dimension = 0; }
	 	OPEN_BRACKET (dim:INT { dimension = Integer.parseInt(dim.getText()); }
	 	              | ID { dimension = 0; })?
	 		CLOSE_BRACKET
			{ decl = array(decl, dimension); }
	)*
	;
  
ptr_operator returns [ Pointer p ]
	{ p = new Pointer(); p.cv = 0; int cvflags = 0; }
	:
  ( STAR     { p.nodeType = Type.TypeNode.NODE_POINTER; }
  |	AMPERSAND { p.nodeType = Type.TypeNode.NODE_REFERENCE; }
  | (QUAD)? nested_name_specifier STAR 
  	          { p.nodeType = Type.TypeNode.NODE_POINTER; } 
  )
    (cvflags=cv { p.cv |= cvflags; })*
	;

parameter_list:
	( parameter ( COMMA parameter )* )?
	;

parameter:
	typeexpr
	;
	
declarator_id:
	((QUAD)? nested_name_specifier) => (QUAD)? nested_name_specifier type_name
  |	id_expression
	;

template_id returns [ Type.TypeNode node=m_err ]
	{ Entity template = null; String tname; List args; }
	:
	tname=template_name { template = peek(tname); }
	OPEN_ANGLE args=template_argument_list CLOSE_ANGLE
		{ node = new Type.TypeNode(Type.TypeNode.NODE_TEMPLATE_INSTANTIATION);
		  node.add(new Type.TypeNode(template));
		  for (Iterator argi = args.iterator(); argi.hasNext(); ) {
		  	node.add(new DefaultMutableTreeNode(argi.next()));
		  }
		}
	;

template_name returns [ String s="?" ]:
	s=nested_name
	;

template_argument_list returns [ List args ]
	{ TemplateArgument arg; args = new LinkedList(); }
	:
	arg=template_argument { if (arg!= null) args.add(arg); }
		( COMMA arg=template_argument { args.add(arg); } )*
	;

template_argument returns [ TemplateArgument arg=null ]
	{ String data; Type.TypeNode type; }
	:
	data=assignment_expression { arg = new DataTemplateArgument(data); }
  |	type=typeexpr { arg = new TypenameTemplateArgument(new Type(type)); }
	;

cv returns [ int flag=0 ]:
	"const" { flag = Specifiers.CVQualifiers.CONST; }
  |	"volatile" { flag = Specifiers.CVQualifiers.VOLATILE; }
	;

/*
type_id:
	(type_specifier)* declarator
	;
	
type_specifier:
	simple_type_specifier
  |	cv
	;

simple_type_specifier:
	(QUAD)? nested_name_specifier type_name
  |	basic_type ;
*/

basic_type returns [ Type.TypeNode type=m_err ] 
	{ String base = "int"; String sign = ""; }
	:
  (
	(   "signed"         { sign = "signed "; }
	  | "unsigned"       { sign = "unsigned "; }
	)
	(   "char"               { base = "char"; }
      | "short" ("int")?     { base = "short"; }
      | "int"                { base = "int"; }
      | "long"               { base = "long"; }
  	    (  "int"
  	     | "double"          { base = "long double"; }
  	     | "long" ("int")?   { base = "long long"; }
  	    )?
  	)?
  |
    (   "char"               { base = "char"; }
      | "wchar"              { base = "wchar"; }
      | "bool"               { base = "bool"; }
      | "short" ("int")?     { base = "short"; }
      | "int"                { base = "int"; }
      | "long"               { base = "long"; }
        (  "int"
         | "double"          { base = "long double"; }
         | "long" ("int")?   { base = "long long"; }
        )?
      | "float"              { base = "float"; }
      | "double"             { base = "double"; }
      | "void"               { base = "void"; }
    )
  )
  		{ type = new Type.TypeNode(primitive(sign + base)); }

 |	ELLIPSIS { type = ellipsis(); }
	;
	
nested_name_specifier:
	( options{greedy=true;}: ID QUAD )+
	;

nested_name returns [ String s="" ]
	{ StringBuffer sb = new StringBuffer(); }
	:
	(QUAD { sb.append("::"); } )? t0:ID { sb.append(t0.getText()); }
		( options{greedy=true;}:
		  QUAD t1:ID { sb.append("::" + t1.getText()); } )*
			{ s = sb.toString(); }
	;
	
ignored: "virtual" | "MUTABLE" | "register" ;


/* Pruned branches */
id_expression: ID ;
assignment_expression returns [String txt=""]: t:INT { txt = t.getText(); };
constant_expression: INT ;
type_name: ID ;
