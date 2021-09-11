# WordCount
A WordCount project with OpenMPI.
This is the final submission of the Concurrent, Parallel and Cloud Programming class at the University of Salerno.
The goal is to implement a word count parallel solution, that works on a directory of files and is deployed on a commodity cluster. 
The entire directory of files is iterated to take count of every word inside the files. At the end of the counting, a partition is created for each file, and every slave gets the assigned portion.
The OpenMPI library is available for C language and many others, but in this case it was necessary to use the C language and to deploy the project on a AWS cluster, consisting of 
8 instances of x2large machines.
The approach for the final solution is not a sequential approach, but, indeed, a concurrent, distributed, approach. 
Speedup, strong and weak scalability are the main aspects to optimize during the testing and implementation of the solution.
Test test files are not included, and the solution is not fully optimal due to a problem in the preprocessing of the files.
