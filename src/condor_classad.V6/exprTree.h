/***************************Copyright-DO-NOT-REMOVE-THIS-LINE**
 * CONDOR Copyright Notice
 *
 * See LICENSE.TXT for additional notices and disclaimers.
 *
 * Copyright (c)1990-1998 CONDOR Team, Computer Sciences Department, 
 * University of Wisconsin-Madison, Madison, WI.  All Rights Reserved.  
 * No use of the CONDOR Software Program Source Code is authorized 
 * without the express consent of the CONDOR Team.  For more information 
 * contact: CONDOR Team, Attention: Professor Miron Livny, 
 * 7367 Computer Sciences, 1210 W. Dayton St., Madison, WI 53706-1685, 
 * (608) 262-0856 or miron@cs.wisc.edu.
 *
 * U.S. Government Rights Restrictions: Use, duplication, or disclosure 
 * by the U.S. Government is subject to restrictions as set forth in 
 * subparagraph (c)(1)(ii) of The Rights in Technical Data and Computer 
 * Software clause at DFARS 252.227-7013 or subparagraphs (c)(1) and 
 * (2) of Commercial Computer Software-Restricted Rights at 48 CFR 
 * 52.227-19, as applicable, CONDOR Team, Attention: Professor Miron 
 * Livny, 7367 Computer Sciences, 1210 W. Dayton St., Madison, 
 * WI 53706-1685, (608) 262-0856 or miron@cs.wisc.edu.
****************************Copyright-DO-NOT-REMOVE-THIS-LINE**/

#ifndef __EXPR_TREE_H__
#define __EXPR_TREE_H__

#include "common.h"
#include "value.h"

// forward declarations
class ExprTree;
class ClassAd;
class Source;
class Sink;

// structures pertaining to the classad domain
#include "domain.h"

/// The kinds of nodes in expression trees 
enum NodeKind 
{
	/** Literal node (string, integer, real, boolean, undefined, error) */
																LITERAL_NODE,
	/** Attribute reference node (attr, .attr, expr.attr) */	ATTRREF_NODE,
	/** Expression operation node (unary, binary, ternary) */ 	OP_NODE,
	/** Function call node */									FN_CALL_NODE,
	/** ClassAd node */											CLASSAD_NODE,
	/** Expression list node */									EXPR_LIST_NODE
};

class EvalState {
	public:
		EvalState( );

		void setRootScope( );
		void setScopes( ClassAd* );

		HashTable<ExprTree*,Value> cache;
		ClassAd	*rootAd;
		ClassAd *curAd;
};


/** A node of the expression tree, which may be a literal, attribute reference,
	function call, classad, expression list, or an operator applied to other
	ExprTree operands.
	@see NodeKind
*/
class ExprTree 
{
  	public:
		/// Virtual destructor
		virtual ~ExprTree ();

		/** Factory method to parse an expression.
			@param s The Source object.
			@return The expression, or NULL on parse failure.
			@see Source
		*/
		static ExprTree *fromSource (Source &s);

		/** Static method to set the default copy mode when copying	
				expressions (deep copies or reference counted copies.)  This
				is an experimental feature; its use is discouraged.
			@param c The mode to be used for copying.
		*/
		static void setDefaultCopyMode( CopyMode c );
		
		/** Sets the lexical parent scope of the expression, which is used to 
				determine the lexical scoping structure for resolving attribute
				references. (However, the semantic parent may be different from 
				the lexical parent if a {\tt super} attribute is specified.) 
				This method is automatically called when expressions are 
				inserted into ClassAds, and should thus be called explicitly 
				only when evaluating expressions which haven't been inserted 
				into a ClassAd.
			@param p The parent ClassAd.
		*/
		void setParentScope( ClassAd* p );

		/** Sends the expression object to a Sink.
			@param s The Sink object.
			@return false if the expression could not be successfully
				unparsed into the sink.
			@see Sink
		*/
		virtual bool toSink (Sink &s)  = 0;

		/** Makes a copy of the expression tree.  Explicitly specifying the 
				copy mode is an experimental feature; its use is discouraged.
			@param c The copy mode to be used (i.e., deep copy, reference 
				counting, or the default copy mode).
			@return The copy of the expression, or NULL if the copy failed.
			@see setDefaultCopyMode, CopyMode
		*/
		ExprTree *copy( CopyMode c=EXPR_DEFAULT_COPY );

		/** Gets the node kind of this expression node.
			@return The node kind.
			@see NodeKind
		*/
		NodeKind getKind (void) { return nodeKind; }


		/** Evaluates the expression.
			@param v The value of the expression.
			@return true if the operation succeeded, and false otherwise.
		*/
		bool evaluate( Value& v );

		/** Evaluates the expression and identifies significant sub-expressions
				that determined the value of the expression.
			@param v The value of the expression.
			@param t The expression composed of the significant sub-expressions.
			@return true if the operation succeeded, and false otherwise.
		*/
		bool evaluate( Value& v, ExprTree*& t );

		/** Performs a ``partial evaluation'' of the expression.  This method 
				evaluates as much of the expression as possible without 
				resolving ``external references.''  It is roughly equivalent 
				to expression simplification by constant propagation and 
				folding.
			@param val Contains the value of the expression if it could be 
				completely folded.  The value of this object is valid if and
				only if tree is NULL.
			@param tree Contains the flattened expression tree if the expression
				could not be fully flattened to a value, and NULL otherwise.
			@return true if the flattening was successful, and false otherwise.
		*/
		bool flatten( Value& val, ExprTree*& tree);

		
  	protected:
		ExprTree ();

		bool flatten( EvalState&, Value&, ExprTree*&, OpKind* = NULL );
		bool evaluate( EvalState &, Value & ); 
		bool evaluate( EvalState &, Value &, ExprTree *& );

		static CopyMode	defaultCopyMode;
		ClassAd		*parentScope;
		NodeKind	nodeKind;
		int			refCount;

  	private:
		friend class Operation;
		friend class AttributeReference;
		friend class FunctionCall;
		friend class FunctionTable;
		friend class ExprList;
		friend class ExprListIterator;
		friend class ClassAd; 

		virtual ExprTree *_copy( CopyMode = EXPR_DEEP_COPY )=0;
		virtual void _setParentScope( ClassAd* )=0;
		virtual bool _evaluate( EvalState&, Value& )=0;
		virtual bool _evaluate( EvalState&, Value&, ExprTree*& )=0;
		virtual bool _flatten( EvalState&, Value&, ExprTree*&, OpKind* )=0;

		bool flattenFlag;
};


#include "literals.h"
#include "attrrefs.h"
#include "operators.h"
#include "fnCall.h"
#include "exprList.h"
#include "classad.h"

#endif//__EXPR_TREE_H__
