## Build Instructions

* On Mac OS X, ```xcodebuild``` will generate a command-line binary
* On Linux/BSD/etc. ```make``` should suffice

The code has been tested with _clang_ and _GCC 4.9_ and passes the [Google C++ Style Guide](https://raw.githubusercontent.com/google/styleguide/gh-pages/cpplint/cpplint.py)

## Please supply us with your source code in any language you are comfortable

It would have been relatively straightforward to write the code in Python, but due to the emphasis on performance in the question, C++11 seemed more efficient.  The main advantage is that the standard libraries provide data structures that are significantly faster and more efficient than the bundled Python data types.  

For example, the most likely data store for this  project would have been Python’s tuple type, but that invokes an O(n) penalty on resize, which happens quite frequently in this exercise.  [Python Complexity](https://www.ics.uci.edu/~pattis/ICS-33/lectures/complexitypython.txt) A sort is an O(n log n). In the next questions, I address the complexity of the STL operations which should make the choice more clear.  

## What is the time complexity for processing an Add Order message?
Adding an element to the end of a STL vector is a O(1) operation, however since calculating the price requires a sort, the end result is a O(log n) operation. [STL Reference](http://john-ahlgren.blogspot.com/2013/10/stl-container-performance.html)

##What is the time complexity for processing a Reduce Order message?
The reduce order operation uses a ```std::map``` which incurs a O(1) for the lookup and a O(log n) for the actual reduction.  

##If your implementation were put into production and found to be too slow, what ideas would you try out to improve its performance?
As it stands, much of the more intensive calculations are being passed to the compiler, the biggest limiting factor here is that vector data is less memory efficient than other stores. On the other hand, we are primarily using 32-bit variables (t_int32) and there is some argument that aligning variables on memory boundaries in line with the CPU architecture can make a speed difference at a greater memory cost.

So, there are two primary scenarios that could be optimized without redesigning the algorithm.

1. If we are RAM limited, an alternate data type (```std::unordered_map```) would be more RAM efficient at the cost of greater CPU usage.
2. If we are CPU limited, but have a lot of RAM, switching to ```t_int64``` variables would be faster. 

In the end, there are no major loops in the code that need to be optimized so a faster implementation would require a rethink of how the process is undertaken. Currently, there is no parallelism in how the process works. A given data set is processed as new input is recieved.  

One potential area of improvement would be to move the code into separate threads for input, pricing and output though in the end, the output result is still dependent on the ```price()``` function completing. 

##Additional Comments
_The data set in the document is missing a line from the feed.txt that does materially change the results (```28815774 R k 100```) If that line is included, the sample data in the document produces the output requested._
