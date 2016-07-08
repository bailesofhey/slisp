# slisp
Modern LISP variant inspired by Python, Clojure & Arc

## Background

Slisp is written in modern C++. I considered writing it in C, C#, Java, Python or another lisp. C++ was chosen for a variety of reasons I'll elaborate on in another article.

Slisp is still in its infancy - it's got a long way to go before it becomes "stable".

## Design Principles

The following design principles describe how I'd like to see Slisp evolve over the next few years - not what's current possible in it. 

### A Modern Lisp

LISP is one of oldest high level programming languages. It might be the most influential language of all time - every year programming becomes more and more lisp-esque. However, it is natural for languages to evolve over time as the problems that need to be solved change, hardware changes, culture changes. Languages such as Clojure and Arc are new sources of innovation, controversially questioning the design decisions on their predecessors. It's high time for Lisp to make a come back. My ultimate goal is for Slisp to become a new source of inspiration, fueling the fire for this renaissance. 

## Don't be afraid to add syntax

This might be the most controversial design choice. To many, the defining characteristic of lisp *is* the lack of syntax. With a uniform syntax for data and logic, lisp macros can transform code just as easily as regular lisp code transforms data. So, by adding more syntax to lisp many claim, you make writing macros more difficult. I don't think this is necessarily the case. As long as, at the end of the day, all newly introduced syntax expand into the familiar s-exp, new syntax simply becomes shorthand for a more verbose s-exp. New syntax introduced should be optional - use it if you want to.

## Don't Object to Object Orientation

### Scheme
Scheme doesn't have a system for OO builtin (but you can roll your own, incomplete, incompatible, bug ridden one yourself as a weekend project). IMHO, a well designed facility for OO is important enough to be builtin to the platform itself. This is so that there isn't a proliferation of many, many ways to accomplish the same thing (there's that Python philosophy shining through).

### Arc and Clojure 
Both shun OO as overused, unnecessary and in conflict with functional programming (shared mutable state is the sworn enemy of concurrency). While some of the arguments may be true, good object oriented design can bring a lot good too, especially when it comes to large software projects consisting of dozens of developers working in huge code bases (hundreds of thousands of lines of code or more). Whether you subscribe to pure FP, pure OO or somewhere in the middle, Slisp will provide a system for doing OO anyway, use it if you want to.

### Common Lisp
CL has CLOS which might be the most powerful OO platform ever constructed. Slisp's facilities for OO will probably never be as powerful as CLOS. However, if done properly it might be simpler to use and more comprehensible. We'll see (it hasn't been built yet!).

## Static and Dynamically Typed
The holy wars between static and dynamic typing zealots will rage on for decades to come. Meanwhile, the rest of the world realized a long time ago that *both* have theirs strengths and weakenesses - so the only thing to really argue about is what's the best combination to use, given the problem at hand. Slisp will strive to make it easy to be pure dynamic, pure static, or a combination if you chose. 

## Solid
Slisp strives to be of high code quality, well designed (using the principles of OO) and well tested (Slisp is written using TDD). 

## Tweakable
I've tried to write Slisp in a way that maximizes swability without increasing complexity. If you want to swap of the Standard Library for your own, you should be able to it.
