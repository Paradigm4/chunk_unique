/*
 *    _____      _ ____  ____
 *   / ___/_____(_) __ \/ __ )
 *   \__ \/ ___/ / / / / __  |
 *  ___/ / /__/ / /_/ / /_/ / 
 * /____/\___/_/_____/_____/  
 *
 *
 * BEGIN_COPYRIGHT
 *
 * This file is part of SciDB.
 * Copyright (C) 2008-2014 SciDB, Inc.
 *
 * SciDB is free software: you can redistribute it and/or modify
 * it under the terms of the AFFERO GNU General Public License as published by
 * the Free Software Foundation.
 *
 * SciDB is distributed "AS-IS" AND WITHOUT ANY WARRANTY OF ANY KIND,
 * INCLUDING ANY IMPLIED WARRANTY OF MERCHANTABILITY,
 * NON-INFRINGEMENT, OR FITNESS FOR A PARTICULAR PURPOSE. See
 * the AFFERO GNU General Public License for the complete license terms.
 *
 * You should have received a copy of the AFFERO GNU General Public License
 * along with SciDB.  If not, see <http://www.gnu.org/licenses/agpl-3.0.html>
 *
 * END_COPYRIGHT
 */
#include "query/Operator.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int
cmpstringp (const void *p1, const void *p2)
{

/* The actual arguments to this function are "pointers to pointers to char",
 *  * but strcmp(3) arguments are "pointers to char", hence the following cast
 *   * plus dereference */

  return strcmp (*(char *const *) p1, *(char *const *) p2);
}

namespace scidb
{

class Physicalcu : public PhysicalOperator
{
public:
    Physicalcu(string const& logicalName,
                string const& physicalName,
                Parameters const& parameters,
                ArrayDesc const& schema):
        PhysicalOperator(logicalName, physicalName, parameters, schema)
    {}

    virtual ArrayDistribution getOutputDistribution(vector<ArrayDistribution> const& inputDistributions,
                                                    vector<ArrayDesc> const& inputSchemas) const
    {
       return inputDistributions[0];
    }

    /**
      * [Optimizer API] Determine if operator changes result chunk distribution.
      * @param sourceSchemas shapes of all arrays that will given as inputs.
      * @return true if will changes output chunk distribution, false if otherwise
      */
    virtual bool changesDistribution(std::vector<ArrayDesc> const& sourceSchemas) const
    {
        return false;
    }

    /* The instance-parallel 'main' routine of this operator.
       This runs on each instance in the SciDB cluster.
     */
    shared_ptr< Array> execute(vector< shared_ptr< Array> >& inputArrays, shared_ptr<Query> query)
    {
        shared_ptr<Array> inputArray = inputArrays[0];
        shared_ptr<ConstArrayIterator> arrayIter(inputArray->getConstIterator(0));
        shared_ptr<ConstChunkIterator> chunkIter;
        shared_ptr<Array> output(new MemArray(inputArray->getArrayDesc(), query));
        shared_ptr<ArrayIterator> outputArrayIterator(output->getIterator(0));

// Iterate over each chunk owned by this instance
        while (!arrayIter->end())
        {
            chunkIter = arrayIter->getChunk().getConstIterator(0);
            Coordinates start = chunkIter->getPosition();
            int64_t last_row = start[0];
            vector<string> chunkdata;
            for(;;)
            {
                if(!chunkIter->end())
                {
                    Value const& val = chunkIter->getItem();
                    Coordinates coords = chunkIter->getPosition();
// XXX this is a dumb way to munge these data, improve
                    chunkdata.push_back(string(val.getString()));
                    last_row = coords[0];
                    ++(*chunkIter);
                }
                if(chunkIter->end()) break;
            }
            const char **a = (const char **)malloc(chunkdata.size() * sizeof(char *));
            if(!a) throw PLUGIN_USER_EXCEPTION("chunk unique malloc error", SCIDB_SE_UDO, SCIDB_USER_ERROR_CODE_START);
            for(unsigned int j=0;j<chunkdata.size(); ++j) a[j] = chunkdata[j].c_str();
            qsort (a, chunkdata.size(), sizeof (char *), cmpstringp);
// write the output (has same schema as input)
            shared_ptr<ChunkIterator> outputChunkIter = outputArrayIterator->newChunk(start).getIterator(query, ChunkIterator::SEQUENTIAL_WRITE);
            size_t j = 0;
            Value val;
            const char *ref = a[0];
            for(;;)
            {
                if(outputChunkIter->end() || j>=chunkdata.size()) break;
                if(a[j] && ((strcmp(ref, a[j])!=0) || j==0))
                {
                    val.setData(a[j],strlen(a[j])+1);
                    ref = a[j];
                    outputChunkIter->writeItem(val);
                }
                ++j;
                ++(*outputChunkIter);
            }
            if(a) free(a);
            outputChunkIter->flush();
            if(outputChunkIter) outputChunkIter->reset();
// Advance the array iterators in lock step
            ++(*arrayIter);
            ++(*outputArrayIterator);
        }
        return output;
    }
};
REGISTER_PHYSICAL_OPERATOR_FACTORY(Physicalcu, "cu", "Physicalcu");
} //namespace scidb
