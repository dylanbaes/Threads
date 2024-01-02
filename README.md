# Multithreading

This project is an experiment that utilizes multithreads in C using pthreads and pid concepts. The project shows the advantages and disdvantages of multithreading and how threads work. It is concluded that there is a certain amount of threads where the runtime improvement starts to flatten and therefore increasing threads is pointless. This project demonstrates this by having a C project that computes and prints out an image using n number of threads.

![image](https://github.com/dylanbaes/Threads/assets/77146078/1d79dfe8-d697-46c9-a19e-bbc483a83f6d)

As shown on the graph for the image Mandel A, the more threads there are, the faster the runtime is, that is until after a certain number of threads are used (in this case 10). This project shows that there is an optimal number of threads that should be used in certain computations, and increasing or decreasing the number of threads may decrease performance.
