1. Describe how your prefetcher works.
This prefetcher uses the sequential aproach as its base strategy, but then if it detects a stride that is repeating, it will use that as its prefetch value. The goal is to take advantage of more of these common strides that could be further apart than the sequential N value. 

4 values are stored in the data of the prefetcher. The first value is the prefetch amount used the same as in the sequential prefether. The second value is the previous address that we accessed. The third is a stride length that can be stored and used/compared against. The fourth value is essentially a boolean that marks if we are in "stride mode" or "standard sequential mode".

When we have a miss, but our calculated stride matches the difference between the current and previous addresses, we then fetch by that stride and put ourselves in stride mode. We will stay in stride mode and fetch by that stride until we have a miss. If the difference does not match, we continue using the sequential aproach.

When we have a hit, we simply use strided if we are in strided mode, or sequential if stride mode is not active.

2. Explain how you chose that prefetch strategy.
My strategy was inspired partially by the paper in the GitHub repository written at the University of Utah. The idea of tracking some state was something I wanted to do in this prefetcher, and it seemed like it would be possible to use this to build on one of the current strategys by providing an optimization. 

3. Discuss the pros and cons of your prefetch strategy. (ADD MORE TO THIS)
One of the pros of this strategy is we will end up using fewer prefetches in this solution than that normal sequential. We will also evict fewer lines in this case as when in "stride mode", we dont evict N lines from the cache, just one. 

The first con of this solution is the prefetcher will take more hardware than the simple sequential method as we will need to add an adder and comparator to decide which set of prefetching to do. We also have to have the sequential prefetcher built in this hardware along with the new hardware as well.

4. Demonstrate that the prefetcher could be implemented in hardware (this can be
   as simple as pointing to an existing hardware prefetcher using the strategy
   or a paper describing a hypothetical hardware prefetcher which implements
   your strategy).

   The hardware that we are using in this prefetcher is similar to the Reference Prediciton Table (RPT) described in the first linked paper on the GitHub Starter Code:
   https://users.cs.utah.edu/~rajeev/cs7810/papers/chen95.pdf

   The prefether will also use the hardware a sequential prefetcher would use.

5. Cite any additional sources that you used to develop your prefetcher.
https://users.cs.utah.edu/~rajeev/cs7810/papers/chen95.pdf
