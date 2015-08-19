/**
 * @file Logicalcu.cpp
 *
 * @brief chunkwise unique
 *
 * @par Synopsis: cu(A).
 *  
 * @par Summary:
 * something
 * <br>
 *
 * @par Input:
 * A is an array with one string attribute
 * <br>
 *
 * @par Output array:
 * Has same schema as A, but is sparse with only within-chunk unique elements
 * <br>
 *
 * @par Examples:
 * See help('cu')
 * <br>
 *
 * @author B. W. Lewis <blewis@paradigm4.com>
 */

using namespace std;

#include "query/Operator.h"
namespace scidb
{

class Logicalcu : public LogicalOperator
{
public:
    Logicalcu(const string& logicalName, const string& alias):
        LogicalOperator(logicalName, alias)
    {
        ADD_PARAM_INPUT()
        _usage = "cu(A)\n"
                 "filter chunk-wise unique elements\n"
                 "where:\n"
                 "  - A is an array with one string attribute\n\n"
"cu(A) returns a sparse array with schema identical to A whose entries contain\n"
"unique elements within each chunk by marking repeated entries empty. The array\n"
"A may be multi-dimensional and chunked arbitrarily. The cu filter is applied\n"
"independently within each chunk. It is intended to help speed up the SciDB uniq\n"
"and sort operators by cutting down on the amount of data to be sorted. Note\n"
"that the order of returned elements may differ from the input array although\n"
"the schema will be the same.\n"
"BASIC EXAMPLE:\n"
"iquery -aq \"load_library('cu')\"\n"
"iquery -aq \"cu(build(<s:string>[i=1:4,4,0],'{1}[(x),(y),(x),(a)]',true))\"\n"
"{i} s   \n"
"{1} 'x' \n"
"{2} 'y' \n"
"{3} 'a' \n"
"{4} 'x' \n\n"
"iquery -aq \"cu(build(<s:string>[i=1:4,4,0],'{1}[(x),(y),(x),(a)]',true))\"\n"
"{i} s  \n"
"{1} 'a'\n"
"{2} 'x'\n"
"{4} 'y'\n\n";
    }

/* inferSchema helps the query planner decide on the shape of
 * the output array. All operators must define this function.
 */
    ArrayDesc inferSchema(vector< ArrayDesc> schemas, shared_ptr< Query> query)
    {
        ArrayDesc const& matrix = schemas[0];
        if(matrix.getAttributes(true)[0].getType() != TID_STRING)
           throw SYSTEM_EXCEPTION(SCIDB_SE_INTERNAL, SCIDB_LE_ILLEGAL_OPERATION) <<  "cu requires a single string-valued attribute";
        Attributes outputAttributes(matrix.getAttributes());
        Dimensions outputDimensions(matrix.getDimensions());
#ifdef CPP11
        return ArrayDesc(matrix.getName(), outputAttributes, outputDimensions, defaultPartitioning());
#else
        return ArrayDesc(matrix.getName(), outputAttributes, outputDimensions);
#endif
    }
};

REGISTER_LOGICAL_OPERATOR_FACTORY(Logicalcu, "cu");

}
