# cu   (chunkwise unique)

Return unique elements within each chunk by filtering repeated values.


## Synopsis
Usage: `cu(A)`

Filter chunk-wise unique elements where:

  - A is an array with one string attribute

## Return value

`cu(A)` returns a sparse array with schema identical to A whose
entries contain unique elements within each chunk by
marking repeated entries empty. The array A may be
multi-dimensional and chunked arbitrarily. The cu filter is
applied independently within each chunk. It is intended to
help speed up the SciDB uniq and sort operators by cutting
down on the amount of data to be sorted. Note that the order
of returned elements may differ from the input array although
the schema will be the same.


## Example
```
iquery -aq "load_library('cu')"

iquery -aq "cu(build(<s:string>[i=1:4,4,0],'{1}[(x),(y),(x),(a)]',true))"
{i} s
{1} 'x'
{2} 'y'
{3} 'a'
{4} 'x'

iquery -aq "cu(build(<s:string>[i=1:4,4,0],\'{1}[(x),(y),(x),(a)]\',true))"
{i} s
{1} 'a'
{2} 'x'
{4} 'y'
```

The `cu` operator can help speed up the SciDB uniq operator workflow quite a bit.
Consider the following timing results run on a 64-instance SciDB cluster comparing
the usual `uniq(sort(...))` approach with and without `cu`:
```
time iquery -aq "op_count(uniq(sort(build(<s:string>[i=1:100000000,1000000,0],i % 5000))))"
{i} count
{0} 5000

real    2m34.963s
user    0m0.017s
sys     0m0.005s

time iquery -aq "op_count(uniq(sort(cu(build(<s:string>[i=1:100000000,1000000,0],i % 5000)))))"
{i} count
{0} 5000

real    0m10.777s
user    0m0.013s
sys     0m0.008s
```
   

## Installing the plug in

You'll need SciDB installed, along with the SciDB development header packages.
The names vary depending on your operating system type, but they are the
package that have "-dev" in the name. You *don't* need the SciDB source code to
compile and install this.

Run `make` and copy  the `*.so` plugin to the `lib/scidb/plugins`
directory on each of your SciDB cluster nodes. Here is an example:

```
cd chunk_unique
make
cp *.so /opt/scidb/14.8/lib/scidb/plugins

iquery -aq "load_library('cu')"
```
Remember to copy the plugin to all your SciDB cluster nodes.

