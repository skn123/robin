// $ANTLR 2.7.5 (20050128): "src/griffin/sourceanalysis/dox/TypeExpression.g" -> "TypeExpressionParser.java"$

package sourceanalysis.dox;

import antlr.TokenBuffer;
import antlr.TokenStreamException;
import antlr.TokenStreamIOException;
import antlr.ANTLRException;
import antlr.LLkParser;
import antlr.Token;
import antlr.TokenStream;
import antlr.RecognitionException;
import antlr.NoViableAltException;
import antlr.MismatchedTokenException;
import antlr.SemanticException;
import antlr.ParserSharedInputState;
import antlr.collections.impl.BitSet;

import sourceanalysis.*;
import java.util.List;
import java.util.LinkedList;
import java.util.Iterator;

import javax.swing.tree.DefaultMutableTreeNode;


public class TypeExpressionParser extends antlr.LLkParser       implements TypeExpressionLexerTokenTypes
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
	
	private StringBuffer m_error_msg;

protected TypeExpressionParser(TokenBuffer tokenBuf, int k) {
  super(tokenBuf,k);
  tokenNames = _tokenNames;
}

public TypeExpressionParser(TokenBuffer tokenBuf) {
  this(tokenBuf,1);
}

protected TypeExpressionParser(TokenStream lexer, int k) {
  super(lexer,k);
  tokenNames = _tokenNames;
}

public TypeExpressionParser(TokenStream lexer) {
  this(lexer,1);
}

public TypeExpressionParser(ParserSharedInputState state) {
  super(state,1);
  tokenNames = _tokenNames;
}

	public final sourceanalysis.Type.TypeNode  typeexpr() throws RecognitionException, TokenStreamException {
		 sourceanalysis.Type.TypeNode node=m_err ;
		
		Type.TypeNode base = m_err;
		
		try {      // for error handling
			base=basename();
			node=declarator(base);
			{
			switch ( LA(1)) {
			case EOF:
			{
				match(Token.EOF_TYPE);
				break;
			}
			case CLOSE_PAREN:
			case CLOSE_ANGLE:
			case COMMA:
			{
				break;
			}
			default:
			{
				throw new NoViableAltException(LT(1), getFilename());
			}
			}
			}
		}
		catch (RecognitionException ex) {
			if (inputState.guessing==0) {
				reportError(ex);
				recover(ex,_tokenSet_0);
			} else {
			  throw ex;
			}
		}
		return node;
	}
	
	public final sourceanalysis.Type.TypeNode  basename() throws RecognitionException, TokenStreamException {
		 sourceanalysis.Type.TypeNode node=m_err ;
		
		String namestring; int cvflags = Specifiers.CVQualifiers.NONE; int flag=0;
		
		try {      // for error handling
			{
			_loop29:
			do {
				if ((LA(1)==LITERAL_const||LA(1)==LITERAL_volatile)) {
					flag=cv();
					if ( inputState.guessing==0 ) {
						cvflags |= flag;
					}
				}
				else {
					break _loop29;
				}
				
			} while (true);
			}
			{
			boolean synPredMatched32 = false;
			if (((LA(1)==ID||LA(1)==QUAD))) {
				int _m32 = mark();
				synPredMatched32 = true;
				inputState.guessing++;
				try {
					{
					template_id();
					}
				}
				catch (RecognitionException pe) {
					synPredMatched32 = false;
				}
				rewind(_m32);
				inputState.guessing--;
			}
			if ( synPredMatched32 ) {
				node=template_id();
			}
			else if ((LA(1)==ID||LA(1)==QUAD)) {
				namestring=nested_name();
				if ( inputState.guessing==0 ) {
					node = new Type.TypeNode(peek(namestring));
				}
			}
			else if ((_tokenSet_1.member(LA(1)))) {
				node=basic_type();
			}
			else {
				throw new NoViableAltException(LT(1), getFilename());
			}
			
			}
			{
			boolean synPredMatched35 = false;
			if (((LA(1)==QUAD))) {
				int _m35 = mark();
				synPredMatched35 = true;
				inputState.guessing++;
				try {
					{
					match(QUAD);
					}
				}
				catch (RecognitionException pe) {
					synPredMatched35 = false;
				}
				rewind(_m35);
				inputState.guessing--;
			}
			if ( synPredMatched35 ) {
				{
				match(QUAD);
				namestring=nested_name();
				if ( inputState.guessing==0 ) {
					node = new Type.TypeNode(peek(node.formatCpp() + "::" + namestring));
				}
				}
			}
			else if ((_tokenSet_2.member(LA(1)))) {
			}
			else {
				throw new NoViableAltException(LT(1), getFilename());
			}
			
			}
			if ( inputState.guessing==0 ) {
				if (node == m_err) node = errorType(); node.setCV(cvflags);
			}
		}
		catch (RecognitionException ex) {
			if (inputState.guessing==0) {
				reportError(ex);
				recover(ex,_tokenSet_2);
			} else {
			  throw ex;
			}
		}
		return node;
	}
	
	public final Type.TypeNode  declarator(
		Type.TypeNode base
	) throws RecognitionException, TokenStreamException {
		Type.TypeNode decl=m_err;
		
		Token  dim = null;
		int dimension = 0;
		
		try {      // for error handling
			decl=direct_declarator(base);
			{
			_loop47:
			do {
				switch ( LA(1)) {
				case OPEN_PAREN:
				{
					match(OPEN_PAREN);
					parameter_list();
					match(CLOSE_PAREN);
					if ( inputState.guessing==0 ) {
						decl = func(decl);
					}
					break;
				}
				case OPEN_BRACKET:
				{
					if ( inputState.guessing==0 ) {
						dimension = 0;
					}
					match(OPEN_BRACKET);
					{
					switch ( LA(1)) {
					case INT:
					{
						dim = LT(1);
						match(INT);
						if ( inputState.guessing==0 ) {
							dimension = Integer.parseInt(dim.getText());
						}
						break;
					}
					case CLOSE_BRACKET:
					{
						break;
					}
					default:
					{
						throw new NoViableAltException(LT(1), getFilename());
					}
					}
					}
					match(CLOSE_BRACKET);
					if ( inputState.guessing==0 ) {
						decl = array(decl, dimension);
					}
					break;
				}
				default:
				{
					break _loop47;
				}
				}
			} while (true);
			}
		}
		catch (RecognitionException ex) {
			if (inputState.guessing==0) {
				reportError(ex);
				recover(ex,_tokenSet_3);
			} else {
			  throw ex;
			}
		}
		return decl;
	}
	
	public final int  cv() throws RecognitionException, TokenStreamException {
		 int flag=0 ;
		
		
		try {      // for error handling
			switch ( LA(1)) {
			case LITERAL_const:
			{
				match(LITERAL_const);
				if ( inputState.guessing==0 ) {
					flag = Specifiers.CVQualifiers.CONST;
				}
				break;
			}
			case LITERAL_volatile:
			{
				match(LITERAL_volatile);
				if ( inputState.guessing==0 ) {
					flag = Specifiers.CVQualifiers.VOLATILE;
				}
				break;
			}
			default:
			{
				throw new NoViableAltException(LT(1), getFilename());
			}
			}
		}
		catch (RecognitionException ex) {
			if (inputState.guessing==0) {
				reportError(ex);
				recover(ex,_tokenSet_4);
			} else {
			  throw ex;
			}
		}
		return flag;
	}
	
	public final Type.TypeNode  template_id() throws RecognitionException, TokenStreamException {
		 Type.TypeNode node=m_err ;
		
		Entity template = null; String tname; List args;
		
		try {      // for error handling
			tname=template_name();
			if ( inputState.guessing==0 ) {
				template = peek(tname);
			}
			match(OPEN_ANGLE);
			args=template_argument_list();
			match(CLOSE_ANGLE);
			if ( inputState.guessing==0 ) {
				node = new Type.TypeNode(Type.TypeNode.NODE_TEMPLATE_INSTANTIATION);
						  node.add(new Type.TypeNode(template));
						  for (Iterator argi = args.iterator(); argi.hasNext(); ) {
						  	node.add(new DefaultMutableTreeNode(argi.next()));
						  }
						
			}
		}
		catch (RecognitionException ex) {
			if (inputState.guessing==0) {
				reportError(ex);
				recover(ex,_tokenSet_2);
			} else {
			  throw ex;
			}
		}
		return node;
	}
	
	public final String  nested_name() throws RecognitionException, TokenStreamException {
		 String s="" ;
		
		Token  t0 = null;
		Token  t1 = null;
		StringBuffer sb = new StringBuffer();
		
		try {      // for error handling
			{
			switch ( LA(1)) {
			case QUAD:
			{
				match(QUAD);
				if ( inputState.guessing==0 ) {
					sb.append("::");
				}
				break;
			}
			case ID:
			{
				break;
			}
			default:
			{
				throw new NoViableAltException(LT(1), getFilename());
			}
			}
			}
			t0 = LT(1);
			match(ID);
			if ( inputState.guessing==0 ) {
				sb.append(t0.getText());
			}
			{
			_loop84:
			do {
				if ((LA(1)==QUAD)) {
					match(QUAD);
					t1 = LT(1);
					match(ID);
					if ( inputState.guessing==0 ) {
						sb.append("::" + t1.getText());
					}
				}
				else {
					break _loop84;
				}
				
			} while (true);
			}
			if ( inputState.guessing==0 ) {
				s = sb.toString();
			}
		}
		catch (RecognitionException ex) {
			if (inputState.guessing==0) {
				reportError(ex);
				recover(ex,_tokenSet_5);
			} else {
			  throw ex;
			}
		}
		return s;
	}
	
	public final Type.TypeNode  basic_type() throws RecognitionException, TokenStreamException {
		 Type.TypeNode type=m_err ;
		
		Entity base = m_error_entity;
		
		try {      // for error handling
			switch ( LA(1)) {
			case LITERAL_char:
			case LITERAL_wchar:
			case LITERAL_bool:
			case LITERAL_short:
			case LITERAL_int:
			case LITERAL_long:
			case LITERAL_signed:
			case LITERAL_unsigned:
			case LITERAL_float:
			case LITERAL_double:
			case LITERAL_void:
			{
				{
				switch ( LA(1)) {
				case LITERAL_char:
				{
					match(LITERAL_char);
					if ( inputState.guessing==0 ) {
						base = primitive("char");
					}
					break;
				}
				case LITERAL_wchar:
				{
					match(LITERAL_wchar);
					if ( inputState.guessing==0 ) {
						base = primitive("wchar");
					}
					break;
				}
				case LITERAL_bool:
				{
					match(LITERAL_bool);
					if ( inputState.guessing==0 ) {
						base = primitive("bool");
					}
					break;
				}
				case LITERAL_short:
				{
					match(LITERAL_short);
					{
					switch ( LA(1)) {
					case LITERAL_int:
					{
						match(LITERAL_int);
						break;
					}
					case EOF:
					case OPEN_PAREN:
					case CLOSE_PAREN:
					case OPEN_BRACKET:
					case CLOSE_ANGLE:
					case STAR:
					case AMPERSAND:
					case COMMA:
					case ID:
					case QUAD:
					{
						break;
					}
					default:
					{
						throw new NoViableAltException(LT(1), getFilename());
					}
					}
					}
					if ( inputState.guessing==0 ) {
						base = primitive("short");
					}
					break;
				}
				case LITERAL_int:
				{
					match(LITERAL_int);
					if ( inputState.guessing==0 ) {
						base = primitive("int");
					}
					break;
				}
				case LITERAL_long:
				{
					match(LITERAL_long);
					if ( inputState.guessing==0 ) {
						base = primitive("long");
					}
					{
					switch ( LA(1)) {
					case LITERAL_int:
					{
						match(LITERAL_int);
						break;
					}
					case LITERAL_long:
					{
						match(LITERAL_long);
						if ( inputState.guessing==0 ) {
							base = primitive("long long");
						}
						break;
					}
					case EOF:
					case OPEN_PAREN:
					case CLOSE_PAREN:
					case OPEN_BRACKET:
					case CLOSE_ANGLE:
					case STAR:
					case AMPERSAND:
					case COMMA:
					case ID:
					case QUAD:
					{
						break;
					}
					default:
					{
						throw new NoViableAltException(LT(1), getFilename());
					}
					}
					}
					break;
				}
				case LITERAL_signed:
				{
					match(LITERAL_signed);
					if ( inputState.guessing==0 ) {
						base = primitive("int");
					}
					{
					switch ( LA(1)) {
					case LITERAL_char:
					{
						match(LITERAL_char);
						if ( inputState.guessing==0 ) {
							base = primitive("signed char");
						}
						break;
					}
					case LITERAL_int:
					{
						match(LITERAL_int);
						break;
					}
					case LITERAL_long:
					{
						match(LITERAL_long);
						if ( inputState.guessing==0 ) {
							base = primitive("long");
						}
						break;
					}
					case LITERAL_short:
					{
						match(LITERAL_short);
						if ( inputState.guessing==0 ) {
							base = primitive("short");
						}
						break;
					}
					case EOF:
					case OPEN_PAREN:
					case CLOSE_PAREN:
					case OPEN_BRACKET:
					case CLOSE_ANGLE:
					case STAR:
					case AMPERSAND:
					case COMMA:
					case ID:
					case QUAD:
					{
						break;
					}
					default:
					{
						throw new NoViableAltException(LT(1), getFilename());
					}
					}
					}
					break;
				}
				case LITERAL_unsigned:
				{
					match(LITERAL_unsigned);
					if ( inputState.guessing==0 ) {
						base = primitive("unsigned int");
					}
					{
					switch ( LA(1)) {
					case LITERAL_char:
					{
						match(LITERAL_char);
						if ( inputState.guessing==0 ) {
							base = primitive("unsigned char");
						}
						break;
					}
					case LITERAL_int:
					{
						match(LITERAL_int);
						break;
					}
					case LITERAL_long:
					{
						match(LITERAL_long);
						if ( inputState.guessing==0 ) {
							base = primitive("unsigned long");
						}
						{
						switch ( LA(1)) {
						case LITERAL_long:
						{
							match(LITERAL_long);
							if ( inputState.guessing==0 ) {
								base = primitive("unsigned long long");
							}
							break;
						}
						case EOF:
						case OPEN_PAREN:
						case CLOSE_PAREN:
						case OPEN_BRACKET:
						case CLOSE_ANGLE:
						case STAR:
						case AMPERSAND:
						case COMMA:
						case ID:
						case QUAD:
						{
							break;
						}
						default:
						{
							throw new NoViableAltException(LT(1), getFilename());
						}
						}
						}
						break;
					}
					case LITERAL_short:
					{
						match(LITERAL_short);
						if ( inputState.guessing==0 ) {
							base = primitive("unsigned short");
						}
						break;
					}
					case EOF:
					case OPEN_PAREN:
					case CLOSE_PAREN:
					case OPEN_BRACKET:
					case CLOSE_ANGLE:
					case STAR:
					case AMPERSAND:
					case COMMA:
					case ID:
					case QUAD:
					{
						break;
					}
					default:
					{
						throw new NoViableAltException(LT(1), getFilename());
					}
					}
					}
					break;
				}
				case LITERAL_float:
				{
					match(LITERAL_float);
					if ( inputState.guessing==0 ) {
						base = primitive("float");
					}
					break;
				}
				case LITERAL_double:
				{
					match(LITERAL_double);
					if ( inputState.guessing==0 ) {
						base = primitive("double");
					}
					break;
				}
				case LITERAL_void:
				{
					match(LITERAL_void);
					if ( inputState.guessing==0 ) {
						base = primitive("void");
					}
					break;
				}
				default:
				{
					throw new NoViableAltException(LT(1), getFilename());
				}
				}
				}
				if ( inputState.guessing==0 ) {
					type = new Type.TypeNode(base);
				}
				break;
			}
			case ELLIPSIS:
			{
				match(ELLIPSIS);
				if ( inputState.guessing==0 ) {
					type = ellipsis();
				}
				break;
			}
			default:
			{
				throw new NoViableAltException(LT(1), getFilename());
			}
			}
		}
		catch (RecognitionException ex) {
			if (inputState.guessing==0) {
				reportError(ex);
				recover(ex,_tokenSet_2);
			} else {
			  throw ex;
			}
		}
		return type;
	}
	
	public final Type.TypeNode  nonempty_direct_declarator(
		Type.TypeNode base
	) throws RecognitionException, TokenStreamException {
		Type.TypeNode decl=base;
		
		Type.TypeNode mid=m_err; Pointer pointer;
		
		try {      // for error handling
			boolean synPredMatched39 = false;
			if (((_tokenSet_6.member(LA(1))))) {
				int _m39 = mark();
				synPredMatched39 = true;
				inputState.guessing++;
				try {
					{
					ptr_operator();
					}
				}
				catch (RecognitionException pe) {
					synPredMatched39 = false;
				}
				rewind(_m39);
				inputState.guessing--;
			}
			if ( synPredMatched39 ) {
				pointer=ptr_operator();
				if ( inputState.guessing==0 ) {
					mid = ptr(base, pointer);
				}
				decl=direct_declarator(mid);
			}
			else {
				boolean synPredMatched41 = false;
				if (((LA(1)==OPEN_PAREN))) {
					int _m41 = mark();
					synPredMatched41 = true;
					inputState.guessing++;
					try {
						{
						match(OPEN_PAREN);
						}
					}
					catch (RecognitionException pe) {
						synPredMatched41 = false;
					}
					rewind(_m41);
					inputState.guessing--;
				}
				if ( synPredMatched41 ) {
					match(OPEN_PAREN);
					mid=declarator(blank());
					match(CLOSE_PAREN);
					if ( inputState.guessing==0 ) {
						decl = new Type.TypeNode(Type.TypeNode.NODE_X_COMPOSITION);
								  decl.add(mid); decl.add(base);
					}
				}
				else if ((LA(1)==ID)) {
					match(ID);
				}
				else {
					throw new NoViableAltException(LT(1), getFilename());
				}
				}
			}
			catch (RecognitionException ex) {
				if (inputState.guessing==0) {
					reportError(ex);
					recover(ex,_tokenSet_7);
				} else {
				  throw ex;
				}
			}
			return decl;
		}
		
	public final  Pointer  ptr_operator() throws RecognitionException, TokenStreamException {
		 Pointer p ;
		
		p = new Pointer();
		
		try {      // for error handling
			switch ( LA(1)) {
			case STAR:
			{
				match(STAR);
				{
				_loop50:
				do {
					if ((LA(1)==LITERAL_const||LA(1)==LITERAL_volatile)) {
						cv();
					}
					else {
						break _loop50;
					}
					
				} while (true);
				}
				if ( inputState.guessing==0 ) {
					p.nodeType = Type.TypeNode.NODE_POINTER;
				}
				break;
			}
			case AMPERSAND:
			{
				match(AMPERSAND);
				if ( inputState.guessing==0 ) {
					p.nodeType = Type.TypeNode.NODE_REFERENCE;
				}
				break;
			}
			case ID:
			case QUAD:
			{
				{
				switch ( LA(1)) {
				case QUAD:
				{
					match(QUAD);
					break;
				}
				case ID:
				{
					break;
				}
				default:
				{
					throw new NoViableAltException(LT(1), getFilename());
				}
				}
				}
				nested_name_specifier();
				match(STAR);
				{
				_loop53:
				do {
					if ((LA(1)==LITERAL_const||LA(1)==LITERAL_volatile)) {
						cv();
					}
					else {
						break _loop53;
					}
					
				} while (true);
				}
				if ( inputState.guessing==0 ) {
					p.nodeType = Type.TypeNode.NODE_POINTER;
				}
				break;
			}
			default:
			{
				throw new NoViableAltException(LT(1), getFilename());
			}
			}
		}
		catch (RecognitionException ex) {
			if (inputState.guessing==0) {
				reportError(ex);
				recover(ex,_tokenSet_2);
			} else {
			  throw ex;
			}
		}
		return p ;
	}
	
	public final Type.TypeNode  direct_declarator(
		Type.TypeNode base
	) throws RecognitionException, TokenStreamException {
		Type.TypeNode decl=base;
		
		
		try {      // for error handling
			{
			if ((_tokenSet_8.member(LA(1)))) {
				decl=nonempty_direct_declarator(base);
			}
			else if ((_tokenSet_7.member(LA(1)))) {
			}
			else {
				throw new NoViableAltException(LT(1), getFilename());
			}
			
			}
		}
		catch (RecognitionException ex) {
			if (inputState.guessing==0) {
				reportError(ex);
				recover(ex,_tokenSet_7);
			} else {
			  throw ex;
			}
		}
		return decl;
	}
	
	public final void parameter_list() throws RecognitionException, TokenStreamException {
		
		
		try {      // for error handling
			{
			switch ( LA(1)) {
			case ID:
			case QUAD:
			case ELLIPSIS:
			case LITERAL_const:
			case LITERAL_volatile:
			case LITERAL_char:
			case LITERAL_wchar:
			case LITERAL_bool:
			case LITERAL_short:
			case LITERAL_int:
			case LITERAL_long:
			case LITERAL_signed:
			case LITERAL_unsigned:
			case LITERAL_float:
			case LITERAL_double:
			case LITERAL_void:
			{
				parameter();
				{
				_loop57:
				do {
					if ((LA(1)==COMMA)) {
						match(COMMA);
						parameter();
					}
					else {
						break _loop57;
					}
					
				} while (true);
				}
				break;
			}
			case CLOSE_PAREN:
			{
				break;
			}
			default:
			{
				throw new NoViableAltException(LT(1), getFilename());
			}
			}
			}
		}
		catch (RecognitionException ex) {
			if (inputState.guessing==0) {
				reportError(ex);
				recover(ex,_tokenSet_9);
			} else {
			  throw ex;
			}
		}
	}
	
	public final void nested_name_specifier() throws RecognitionException, TokenStreamException {
		
		
		try {      // for error handling
			{
			int _cnt80=0;
			_loop80:
			do {
				if ((LA(1)==ID)) {
					match(ID);
					match(QUAD);
				}
				else {
					if ( _cnt80>=1 ) { break _loop80; } else {throw new NoViableAltException(LT(1), getFilename());}
				}
				
				_cnt80++;
			} while (true);
			}
		}
		catch (RecognitionException ex) {
			if (inputState.guessing==0) {
				reportError(ex);
				recover(ex,_tokenSet_10);
			} else {
			  throw ex;
			}
		}
	}
	
	public final void parameter() throws RecognitionException, TokenStreamException {
		
		
		try {      // for error handling
			typeexpr();
		}
		catch (RecognitionException ex) {
			if (inputState.guessing==0) {
				reportError(ex);
				recover(ex,_tokenSet_11);
			} else {
			  throw ex;
			}
		}
	}
	
	public final void declarator_id() throws RecognitionException, TokenStreamException {
		
		
		try {      // for error handling
			boolean synPredMatched62 = false;
			if (((LA(1)==ID||LA(1)==QUAD))) {
				int _m62 = mark();
				synPredMatched62 = true;
				inputState.guessing++;
				try {
					{
					{
					switch ( LA(1)) {
					case QUAD:
					{
						match(QUAD);
						break;
					}
					case ID:
					{
						break;
					}
					default:
					{
						throw new NoViableAltException(LT(1), getFilename());
					}
					}
					}
					nested_name_specifier();
					}
				}
				catch (RecognitionException pe) {
					synPredMatched62 = false;
				}
				rewind(_m62);
				inputState.guessing--;
			}
			if ( synPredMatched62 ) {
				{
				switch ( LA(1)) {
				case QUAD:
				{
					match(QUAD);
					break;
				}
				case ID:
				{
					break;
				}
				default:
				{
					throw new NoViableAltException(LT(1), getFilename());
				}
				}
				}
				nested_name_specifier();
				type_name();
			}
			else if ((LA(1)==ID)) {
				id_expression();
			}
			else {
				throw new NoViableAltException(LT(1), getFilename());
			}
			
		}
		catch (RecognitionException ex) {
			if (inputState.guessing==0) {
				reportError(ex);
				recover(ex,_tokenSet_12);
			} else {
			  throw ex;
			}
		}
	}
	
	public final void type_name() throws RecognitionException, TokenStreamException {
		
		
		try {      // for error handling
			match(ID);
		}
		catch (RecognitionException ex) {
			if (inputState.guessing==0) {
				reportError(ex);
				recover(ex,_tokenSet_12);
			} else {
			  throw ex;
			}
		}
	}
	
	public final void id_expression() throws RecognitionException, TokenStreamException {
		
		
		try {      // for error handling
			match(ID);
		}
		catch (RecognitionException ex) {
			if (inputState.guessing==0) {
				reportError(ex);
				recover(ex,_tokenSet_12);
			} else {
			  throw ex;
			}
		}
	}
	
	public final String  template_name() throws RecognitionException, TokenStreamException {
		 String s="?" ;
		
		
		try {      // for error handling
			s=nested_name();
		}
		catch (RecognitionException ex) {
			if (inputState.guessing==0) {
				reportError(ex);
				recover(ex,_tokenSet_13);
			} else {
			  throw ex;
			}
		}
		return s;
	}
	
	public final  List  template_argument_list() throws RecognitionException, TokenStreamException {
		 List args ;
		
		TemplateArgument arg; args = new LinkedList();
		
		try {      // for error handling
			arg=template_argument();
			if ( inputState.guessing==0 ) {
				if (arg!= null) args.add(arg);
			}
			{
			_loop68:
			do {
				if ((LA(1)==COMMA)) {
					match(COMMA);
					arg=template_argument();
					if ( inputState.guessing==0 ) {
						args.add(arg);
					}
				}
				else {
					break _loop68;
				}
				
			} while (true);
			}
		}
		catch (RecognitionException ex) {
			if (inputState.guessing==0) {
				reportError(ex);
				recover(ex,_tokenSet_14);
			} else {
			  throw ex;
			}
		}
		return args ;
	}
	
	public final TemplateArgument  template_argument() throws RecognitionException, TokenStreamException {
		 TemplateArgument arg=null ;
		
		String data; Type.TypeNode type;
		
		try {      // for error handling
			switch ( LA(1)) {
			case INT:
			{
				data=assignment_expression();
				if ( inputState.guessing==0 ) {
					arg = new DataTemplateArgument(data);
				}
				break;
			}
			case ID:
			case QUAD:
			case ELLIPSIS:
			case LITERAL_const:
			case LITERAL_volatile:
			case LITERAL_char:
			case LITERAL_wchar:
			case LITERAL_bool:
			case LITERAL_short:
			case LITERAL_int:
			case LITERAL_long:
			case LITERAL_signed:
			case LITERAL_unsigned:
			case LITERAL_float:
			case LITERAL_double:
			case LITERAL_void:
			{
				type=typeexpr();
				if ( inputState.guessing==0 ) {
					arg = new TypenameTemplateArgument(new Type(type));
				}
				break;
			}
			default:
			{
				throw new NoViableAltException(LT(1), getFilename());
			}
			}
		}
		catch (RecognitionException ex) {
			if (inputState.guessing==0) {
				reportError(ex);
				recover(ex,_tokenSet_15);
			} else {
			  throw ex;
			}
		}
		return arg;
	}
	
	public final String  assignment_expression() throws RecognitionException, TokenStreamException {
		String txt="";
		
		Token  t = null;
		
		try {      // for error handling
			t = LT(1);
			match(INT);
			if ( inputState.guessing==0 ) {
				txt = t.getText();
			}
		}
		catch (RecognitionException ex) {
			if (inputState.guessing==0) {
				reportError(ex);
				recover(ex,_tokenSet_15);
			} else {
			  throw ex;
			}
		}
		return txt;
	}
	
	public final void ignored() throws RecognitionException, TokenStreamException {
		
		
		try {      // for error handling
			switch ( LA(1)) {
			case LITERAL_virtual:
			{
				match(LITERAL_virtual);
				break;
			}
			case LITERAL_MUTABLE:
			{
				match(LITERAL_MUTABLE);
				break;
			}
			case LITERAL_register:
			{
				match(LITERAL_register);
				break;
			}
			default:
			{
				throw new NoViableAltException(LT(1), getFilename());
			}
			}
		}
		catch (RecognitionException ex) {
			if (inputState.guessing==0) {
				reportError(ex);
				recover(ex,_tokenSet_12);
			} else {
			  throw ex;
			}
		}
	}
	
	public final void constant_expression() throws RecognitionException, TokenStreamException {
		
		
		try {      // for error handling
			match(INT);
		}
		catch (RecognitionException ex) {
			if (inputState.guessing==0) {
				reportError(ex);
				recover(ex,_tokenSet_12);
			} else {
			  throw ex;
			}
		}
	}
	
	
	public static final String[] _tokenNames = {
		"<0>",
		"EOF",
		"<2>",
		"NULL_TREE_LOOKAHEAD",
		"WS",
		"KEY",
		"OPEN_PAREN",
		"CLOSE_PAREN",
		"OPEN_BRACKET",
		"CLOSE_BRACKET",
		"OPEN_ANGLE",
		"CLOSE_ANGLE",
		"STAR",
		"AMPERSAND",
		"COMMA",
		"ID",
		"INT",
		"QUAD",
		"ELLIPSIS",
		"\"const\"",
		"\"volatile\"",
		"\"char\"",
		"\"wchar\"",
		"\"bool\"",
		"\"short\"",
		"\"int\"",
		"\"long\"",
		"\"signed\"",
		"\"unsigned\"",
		"\"float\"",
		"\"double\"",
		"\"void\"",
		"\"virtual\"",
		"\"MUTABLE\"",
		"\"register\""
	};
	
	private static final long[] mk_tokenSet_0() {
		long[] data = { 18560L, 0L};
		return data;
	}
	public static final BitSet _tokenSet_0 = new BitSet(mk_tokenSet_0());
	private static final long[] mk_tokenSet_1() {
		long[] data = { 4293132288L, 0L};
		return data;
	}
	public static final BitSet _tokenSet_1 = new BitSet(mk_tokenSet_1());
	private static final long[] mk_tokenSet_2() {
		long[] data = { 195010L, 0L};
		return data;
	}
	public static final BitSet _tokenSet_2 = new BitSet(mk_tokenSet_2());
	private static final long[] mk_tokenSet_3() {
		long[] data = { 18562L, 0L};
		return data;
	}
	public static final BitSet _tokenSet_3 = new BitSet(mk_tokenSet_3());
	private static final long[] mk_tokenSet_4() {
		long[] data = { 4294900162L, 0L};
		return data;
	}
	public static final BitSet _tokenSet_4 = new BitSet(mk_tokenSet_4());
	private static final long[] mk_tokenSet_5() {
		long[] data = { 196034L, 0L};
		return data;
	}
	public static final BitSet _tokenSet_5 = new BitSet(mk_tokenSet_5());
	private static final long[] mk_tokenSet_6() {
		long[] data = { 176128L, 0L};
		return data;
	}
	public static final BitSet _tokenSet_6 = new BitSet(mk_tokenSet_6());
	private static final long[] mk_tokenSet_7() {
		long[] data = { 18882L, 0L};
		return data;
	}
	public static final BitSet _tokenSet_7 = new BitSet(mk_tokenSet_7());
	private static final long[] mk_tokenSet_8() {
		long[] data = { 176192L, 0L};
		return data;
	}
	public static final BitSet _tokenSet_8 = new BitSet(mk_tokenSet_8());
	private static final long[] mk_tokenSet_9() {
		long[] data = { 128L, 0L};
		return data;
	}
	public static final BitSet _tokenSet_9 = new BitSet(mk_tokenSet_9());
	private static final long[] mk_tokenSet_10() {
		long[] data = { 36864L, 0L};
		return data;
	}
	public static final BitSet _tokenSet_10 = new BitSet(mk_tokenSet_10());
	private static final long[] mk_tokenSet_11() {
		long[] data = { 16512L, 0L};
		return data;
	}
	public static final BitSet _tokenSet_11 = new BitSet(mk_tokenSet_11());
	private static final long[] mk_tokenSet_12() {
		long[] data = { 2L, 0L};
		return data;
	}
	public static final BitSet _tokenSet_12 = new BitSet(mk_tokenSet_12());
	private static final long[] mk_tokenSet_13() {
		long[] data = { 1024L, 0L};
		return data;
	}
	public static final BitSet _tokenSet_13 = new BitSet(mk_tokenSet_13());
	private static final long[] mk_tokenSet_14() {
		long[] data = { 2048L, 0L};
		return data;
	}
	public static final BitSet _tokenSet_14 = new BitSet(mk_tokenSet_14());
	private static final long[] mk_tokenSet_15() {
		long[] data = { 18432L, 0L};
		return data;
	}
	public static final BitSet _tokenSet_15 = new BitSet(mk_tokenSet_15());
	
	}
