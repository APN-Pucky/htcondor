/***************************************************************
 *
 * Copyright (C) 1990-2007, Condor Team, Computer Sciences Department,
 * University of Wisconsin-Madison, WI.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License"); you
 * may not use this file except in compliance with the License.  You may
 * obtain a copy of the License at
 * 
 *    http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ***************************************************************/

#include "condor_common.h"
#include "compat_classad_util.h"
#include "classad_oldnew.h"
#include "string_list.h"
#include "condor_adtypes.h"
#include "classad/classadCache.h" // for CachedExprEnvelope

#include "compat_classad_list.h"
#ifdef _OPENMP
#include <omp.h>
#endif

/* TODO This function needs to be tested.
 */
int Parse(const char*str, MyString &name, classad::ExprTree*& tree, int*pos)
{
	classad::ClassAdParser parser;
	classad::ClassAd *newAd;

		// We don't support the pos argument at the moment.
	if ( pos ) {
		*pos = 0;
	}

		// String escaping is different between new and old ClassAds.
		// We need to convert the escaping from old to new style before
		// handing the expression to the new ClassAds parser.
	std::string newAdStr = "[";
	newAdStr.append( compat_classad::ConvertEscapingOldToNew( str ) );
	newAdStr += "]";
	newAd = parser.ParseClassAd( newAdStr );
	if ( newAd == NULL ) {
		tree = NULL;
		return 1;
	}
	if ( newAd->size() != 1 ) {
		delete newAd;
		tree = NULL;
		return 1;
	}
	
	classad::ClassAd::iterator itr = newAd->begin();
	name = itr->first.c_str();
	tree = itr->second->Copy();
	delete newAd;
	return 0;
}

/* TODO This function needs to be tested.
 */
int ParseClassAdRvalExpr(const char*s, classad::ExprTree*&tree, int*pos)
{
	classad::ClassAdParser parser;
	std::string str = compat_classad::ConvertEscapingOldToNew( s );
	if ( parser.ParseExpression( str, tree, true ) ) {
		return 0;
	} else {
		tree = NULL;
		if ( pos ) {
			*pos = 0;
		}
		return 1;
	}
}

/*
 */
const char *ExprTreeToString( const classad::ExprTree *expr, std::string & buffer )
{
	classad::ClassAdUnParser unparser;
	unparser.SetOldClassAd( true, true );
	unparser.Unparse( buffer, expr );
	return buffer.c_str();
}

const char *ExprTreeToString( const classad::ExprTree *expr )
{
	static std::string buffer;
	buffer = "";
	return ExprTreeToString(expr, buffer);
}

const char * ClassAdValueToString ( const classad::Value & value, std::string & buffer )
{
	classad::ClassAdUnParser unparser;

	unparser.SetOldClassAd( true, true );
	unparser.Unparse( buffer, value );

	return buffer.c_str();
}

const char * ClassAdValueToString ( const classad::Value & value )
{
	static std::string buffer;
	buffer = "";
	return ClassAdValueToString(value, buffer);
}

classad::ExprTree * SkipExprEnvelope(classad::ExprTree * tree) {
	if ( ! tree) return tree;
	classad::ExprTree::NodeKind kind = tree->GetKind();
	if (kind == classad::ExprTree::EXPR_ENVELOPE) {
		return ((classad::CachedExprEnvelope*)tree)->get();
	}
	return tree;
}

classad::ExprTree * SkipExprParens(classad::ExprTree * tree) {
	if ( ! tree) return tree;
	classad::ExprTree::NodeKind kind = tree->GetKind();
	classad::ExprTree * expr = tree;
	if (kind == classad::ExprTree::EXPR_ENVELOPE) {
		expr = ((classad::CachedExprEnvelope*)tree)->get();
		if (expr) tree = expr;
	}

	kind = tree->GetKind();
	while (kind == classad::ExprTree::OP_NODE) {
		classad::ExprTree *e2, *e3;
		classad::Operation::OpKind op;
		((classad::Operation*)tree)->GetComponents(op, expr, e2, e3);
		if ( ! expr || op != classad::Operation::PARENTHESES_OP) break;
		tree = expr;
		kind = tree->GetKind();
	}

	return tree;
}


static classad::ExprTree * wrap_in_parens_if_needed(classad::ExprTree * expr, classad::Operation::OpKind op)
{
	if ( ! expr) return expr;

	classad::ExprTree::NodeKind kind = expr->GetKind();
	if (kind == classad::ExprTree::OP_NODE) {
		classad::Operation::OpKind op1 = ((classad::Operation*)expr)->GetOpKind();
		if (op1 == classad::Operation::PARENTHESES_OP) {
			// no need to insert parends, this already is one.
		} else if (classad::Operation::PrecedenceLevel(op1) < classad::Operation::PrecedenceLevel(op)) {
			expr = classad::Operation::MakeOperation(classad::Operation::PARENTHESES_OP, expr, NULL, NULL);
		}
	}
	return expr;
}

classad::ExprTree * JoinExprTreeCopiesWithOp(classad::Operation::OpKind op, classad::ExprTree * exp1, classad::ExprTree * exp2)
{
	// before we join these into a new tree, we want to skip over the envelope nodes (if any) and copy them.
	if (exp1) {
		exp1 = SkipExprEnvelope(exp1)->Copy();
		exp1 = wrap_in_parens_if_needed(exp1, op);
	}
	if (exp2) {
		exp2 = SkipExprEnvelope(exp2)->Copy();
		exp2 = wrap_in_parens_if_needed(exp2, op);
	}
	
	return classad::Operation::MakeOperation(op, exp1, exp2, NULL);
}

bool ExprTreeIsLiteral(classad::ExprTree * expr, classad::Value & value)
{
	if ( ! expr) return false;

	classad::ExprTree::NodeKind kind = expr->GetKind();
	if (kind == classad::ExprTree::EXPR_ENVELOPE) {
		expr = ((classad::CachedExprEnvelope*)expr)->get();
		if ( ! expr) return false;
		kind = expr->GetKind();
	}

	// dive into parens
	while (kind == classad::ExprTree::OP_NODE) {
		classad::ExprTree *e2, *e3;
		classad::Operation::OpKind op;
		((classad::Operation*)expr)->GetComponents(op, expr, e2, e3);
		if ( ! expr || op != classad::Operation::PARENTHESES_OP) return false;

		kind = expr->GetKind();
	}

	if (kind == classad::ExprTree::LITERAL_NODE) {
		classad::Value::NumberFactor factor;
		((classad::Literal*)expr)->GetComponents(value, factor);
		return true;
	}

	return false;
}

bool ExprTreeIsLiteralNumber(classad::ExprTree * expr, long long & ival)
{
	classad::Value val;
	if ( ! ExprTreeIsLiteral(expr, val)) return false;
	return val.IsNumber(ival);
}

bool ExprTreeIsLiteralNumber(classad::ExprTree * expr, double & rval)
{
	classad::Value val;
	if ( ! ExprTreeIsLiteral(expr, val)) return false;
	return val.IsNumber(rval);
}

bool ExprTreeIsLiteralBool(classad::ExprTree * expr, bool & bval)
{
	classad::Value val;
	if ( ! ExprTreeIsLiteral(expr, val)) return false;
	long long ival;
	if ( !  val.IsNumber(ival)) return false;
	bval = ival != 0;
	return true;
}

bool ExprTreeIsLiteralString(classad::ExprTree * expr, std::string & sval)
{
	classad::Value val;
	if ( ! ExprTreeIsLiteral(expr, val)) return false;
	return val.IsStringValue(sval);
}


#define IS_DOUBLE_TRUE(val) (bool)(int)((val)*100000)

bool EvalBool(compat_classad::ClassAd *ad, const char *constraint)
{
	static classad::ExprTree *tree = NULL;
	static char * saved_constraint = NULL;
	classad::Value result;
	bool constraint_changed = true;
	double doubleVal;
	long long intVal;
	bool boolVal;

	if ( saved_constraint ) {
		if ( strcmp(saved_constraint,constraint) == 0 ) {
			constraint_changed = false;
		}
	}

	if ( constraint_changed ) {
		// constraint has changed, or saved_constraint is NULL
		if ( saved_constraint ) {
			free(saved_constraint);
			saved_constraint = NULL;
		}
		if ( tree ) {
			delete tree;
			tree = NULL;
		}
		classad::ExprTree *tmp_tree = NULL;
		if ( ParseClassAdRvalExpr( constraint, tmp_tree ) != 0 ) {
			dprintf( D_ALWAYS,
				"can't parse constraint: %s\n", constraint );
			return false;
		}
		tree = compat_classad::RemoveExplicitTargetRefs( tmp_tree );
		delete tmp_tree;
		saved_constraint = strdup( constraint );
	}

	// Evaluate constraint with ad in the target scope so that constraints
	// have the same semantics as the collector queries.  --RR
	if ( !EvalExprTree( tree, ad, NULL, result ) ) {
		dprintf( D_ALWAYS, "can't evaluate constraint: %s\n", constraint );
		return false;
	}
	if( result.IsBooleanValue( boolVal ) ) {
		return boolVal;
	} else if( result.IsIntegerValue( intVal ) ) {
		return intVal != 0;
	} else if( result.IsRealValue( doubleVal ) ) {
		return IS_DOUBLE_TRUE(doubleVal);
	}
	dprintf( D_FULLDEBUG, "constraint (%s) does not evaluate to bool\n",
		constraint );
	return false;
}

bool EvalBool(compat_classad::ClassAd *ad, classad::ExprTree *tree)
{
	classad::Value result;
	double doubleVal;
	long long intVal;
	bool boolVal;

	// Evaluate constraint with ad in the target scope so that constraints
	// have the same semantics as the collector queries.  --RR
	if ( !EvalExprTree( tree, ad, NULL, result ) ) {        
		return false;
	}

	if( result.IsBooleanValue( boolVal ) ) {
		return boolVal;
	} else if( result.IsIntegerValue( intVal ) ) {
		return intVal != 0;
	} else if( result.IsRealValue( doubleVal ) ) {
		return IS_DOUBLE_TRUE(doubleVal);
	}

	return false;
}

bool ClassAdsAreSame( compat_classad::ClassAd *ad1, compat_classad::ClassAd * ad2, StringList *ignored_attrs, bool verbose )
{
	classad::ExprTree *ad1_expr, *ad2_expr;
	const char* attr_name;
	ad2->ResetExpr();
	bool found_diff = false;
	while( ad2->NextExpr(attr_name, ad2_expr) && ! found_diff ) {
		if( ignored_attrs && ignored_attrs->contains_anycase(attr_name) ) {
			if( verbose ) {
				dprintf( D_FULLDEBUG, "ClassAdsAreSame(): skipping \"%s\"\n",
						 attr_name );
			}
			continue;
		}
		ad1_expr = ad1->LookupExpr( attr_name );
		if( ! ad1_expr ) {
				// no value for this in ad1, the ad2 value is
				// certainly different
			if( verbose ) {
				dprintf( D_FULLDEBUG, "ClassAdsAreSame(): "
						 "ad2 contains %s and ad1 does not\n", attr_name );
			}
			found_diff = true;
			break;
		}
		if( ad1_expr->SameAs( ad2_expr ) ) {
			if( verbose ) {
				dprintf( D_FULLDEBUG, "ClassAdsAreSame(): value of %s in "
						 "ad1 matches value in ad2\n", attr_name );
			}
		} else {
			if( verbose ) {
				dprintf( D_FULLDEBUG, "ClassAdsAreSame(): value of %s in "
						 "ad1 is different than in ad2\n", attr_name );
			}
			found_diff = true;
			break;
		}
	}
	return ! found_diff;
}

int EvalExprTree( classad::ExprTree *expr, compat_classad::ClassAd *source,
				  compat_classad::ClassAd *target, classad::Value &result )
{
	int rc = TRUE;
	if ( !expr || !source ) {
		return FALSE;
	}

	const classad::ClassAd *old_scope = expr->GetParentScope();
	classad::MatchClassAd *mad = NULL;

	expr->SetParentScope( source );
	if ( target && target != source ) {
		mad = compat_classad::getTheMatchAd( source, target );
	}
	if ( !source->EvaluateExpr( expr, result ) ) {
		rc = FALSE;
	}

	if ( mad ) {
		compat_classad::releaseTheMatchAd();
	}
	expr->SetParentScope( old_scope );

	return rc;
}

bool IsAMatch( compat_classad::ClassAd *ad1, compat_classad::ClassAd *ad2 )
{
	classad::MatchClassAd *mad = compat_classad::getTheMatchAd( ad1, ad2 );

	bool result = mad->symmetricMatch();

	compat_classad::releaseTheMatchAd();
	return result;
}

static classad::MatchClassAd *match_pool = NULL;
static compat_classad::ClassAd *target_pool = NULL;
static std::vector<compat_classad::ClassAd*> *matched_ads = NULL;

bool ParallelIsAMatch(compat_classad::ClassAd *ad1, std::vector<compat_classad::ClassAd*> &candidates, std::vector<compat_classad::ClassAd*> &matches, int threads, bool halfMatch)
{
	int adCount = candidates.size();
	static int cpu_count = 0;
	int current_cpu_count = threads;
	int iterations = 0;
	size_t matched = 0;

	if(cpu_count != current_cpu_count)
	{
		cpu_count = current_cpu_count;
		if(match_pool)
		{
			delete[] match_pool;
			match_pool = NULL;
		}
		if(target_pool)
		{
			delete[] target_pool;
			target_pool = NULL;
		}
		if(matched_ads)
		{
			delete[] matched_ads;
			matched_ads = NULL;
		}
	}

	if(!match_pool)
		match_pool = new classad::MatchClassAd[cpu_count];
	if(!target_pool)
		target_pool = new compat_classad::ClassAd[cpu_count];
	if(!matched_ads)
		matched_ads = new std::vector<compat_classad::ClassAd*>[cpu_count];

	if(!candidates.size())
		return false;

	for(int index = 0; index < cpu_count; index++)
	{
		target_pool[index].CopyFrom(*ad1);
		match_pool[index].ReplaceLeftAd(&(target_pool[index]));
		matched_ads[index].clear();
	}

	iterations = ((candidates.size() - 1) / cpu_count) + 1;

#ifdef _OPENMP
	omp_set_num_threads(cpu_count);
#endif

#pragma omp parallel
	{

#ifdef _OPENMP
		int omp_id = omp_get_thread_num();
#else
		int omp_id = 0;
#endif
		for(int index = 0; index < iterations; index++)
		{
			bool result = false;
			int offset = omp_id + index * cpu_count;
			if(offset >= adCount)
				break;
			compat_classad::ClassAd *ad2 = candidates[offset];

/*
			if(halfMatch)
			{
				char const *my_target_type = target_pool[omp_id].GetTargetTypeName();
				char const *target_type = ad2->GetMyTypeName();
				if( !my_target_type ) {
					my_target_type = "";
				}
				if( !target_type ) {
					target_type = "";
				}
				if( strcasecmp(target_type,my_target_type) &&
					strcasecmp(my_target_type,ANY_ADTYPE) )
				{
					result = false;
					continue;
				}
			}
*/


			match_pool[omp_id].ReplaceRightAd(ad2);
			if ( !compat_classad::ClassAd::m_strictEvaluation )
			{
				target_pool[omp_id].alternateScope = ad2;
				ad2->alternateScope = &(target_pool[omp_id]);
			}
		
			if(halfMatch)
				result = match_pool[omp_id].rightMatchesLeft();
			else
				result = match_pool[omp_id].symmetricMatch();

			match_pool[omp_id].RemoveRightAd();

			if(result)
			{
				matched_ads[omp_id].push_back(ad2);
			}
		}
	}

	for(int index = 0; index < cpu_count; index++)
	{
		match_pool[index].RemoveLeftAd();
		matched += matched_ads[index].size();
	}

	if(matches.capacity() < matched)
		matches.reserve(matched);

	for(int index = 0; index < cpu_count; index++)
	{
		if(matched_ads[index].size())
			matches.insert(matches.end(), matched_ads[index].begin(), matched_ads[index].end());
	}

	return matches.size() > 0;
}

bool IsAHalfMatch( compat_classad::ClassAd *my, compat_classad::ClassAd *target )
{
		// The collector relies on this function to check the target type.
		// Eventually, we should move that check either into the collector
		// or into the requirements expression.
	char const *my_target_type = GetTargetTypeName(*my);
	char const *target_type = GetMyTypeName(*target);
	if( !my_target_type ) {
		my_target_type = "";
	}
	if( !target_type ) {
		target_type = "";
	}
	if( strcasecmp(target_type,my_target_type) &&
		strcasecmp(my_target_type,ANY_ADTYPE) )
	{
		return false;
	}

	classad::MatchClassAd *mad = compat_classad::getTheMatchAd( my, target );

	bool result = mad->rightMatchesLeft();

	compat_classad::releaseTheMatchAd();
	return result;
}

void AttrList_setPublishServerTime( bool publish )
{
	AttrList_setPublishServerTimeMangled( publish );
}

/**************************************************************************
 *
 * Function: AddClassAdXMLFileHeader
 * Purpose:  Print the stuff that should appear at the beginning of an
 *           XML file that contains a series of ClassAds.
 *
 **************************************************************************/
void AddClassAdXMLFileHeader(std::string &buffer)
{
	buffer += "<?xml version=\"1.0\"?>\n";
	buffer += "<!DOCTYPE classads SYSTEM \"classads.dtd\">\n";
	buffer += "<classads>\n";
	return;

}

/**************************************************************************
 *
 * Function: AddClassAdXMLFileFooter
 * Purpose:  Print the stuff that should appear at the end of an XML file
 *           that contains a series of ClassAds.
 *
 **************************************************************************/
void AddClassAdXMLFileFooter(std::string &buffer)
{
	buffer += "</classads>\n";
	return;

}
