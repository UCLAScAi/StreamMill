Stream Mill
===========

The complete data stream management system must support relational streams,
XML streams, and languages more powerful than SQL and XQuery--as required,
e.g., for mining queries and queries for finding patterns in data streams.

Introduction
=============

Efficient support for continuous queries is critical in many application areas, including sensors networks, traffic monitoring and intrus ion detection. The data-intensive nature of these applications, and the fact that they often compare and combine the results of quer ies on data arriving on the wire with the results of queries on info rmation stored in the database provides the strong rationale for current research projects that focus on adapting and extending SQL and its enabling technology to work on data streams. Unfortunately, SQL is facing severe limitations in this new role, which will severely impair the power and generality of SQL-based management systems. A main objective of Stream Mill is to overcome these limitations, and thus achieve a much broader range of usability and effectiveness in its application domain than other stream management systems. In particular, Stream Mill will address the following issues: 

* support for typical sequences queries, e.g. to search for patterns, which SQL does not support

* the loss of expressive power due to the fact that blocking operators are no-longer usable on data streams--- further reducing the limited expressive power of SQL, and 

* many applications use streaming XML data, rather than relational data streams.

We are developing the enabling technology to overcome these proble ms. In particular, Stream Mill unifies the processing of relational streams and XML streams--much in the same way in which research prototypes and commercial DBMSs are now moving to unify the manage ment of relational data and XML data. Data Streams will also support queries on sequences and ordered data, since they are needed on data streams as much as they are on stored sequences. Finally, Str eam Mill will compensate for the loss of expressive power caused by blocking query operators, through powerful new operators that are nonblocking, such as the continuous UDAs of ATLaS. We also exploring techniques for replacing blocking operators with nonblocking ones when these are used in expressing blocking queries.

References
==========

* [Language Manual](http://wis.cs.ucla.edu/old_wis/stream-mill/doc/esl-manual.pdf)
* [System Manual](http://yellowstone.cs.ucla.edu/projects/images/5/5a/StreamMillClassic.pdf)
