#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[])
{
    int fd1 = 1, fd2 = -1;
    char read_buf[100] = {0};
    int ret = -1;
    int i = 10;
    //open file
    fd1 = open("jikuo_wang.txt", O_RDWR | O_APPEND);
    fd2 = open("jikuo_wang.txt", O_RDWR | O_APPEND);

    if ((-1 == fd1 ) || (-1 == fd2)){
        perror("Open file error");
        _exit(-1);
    }
    //continue read content
    while (i)
    {
        memset(read_buf, 0, sizeof(read_buf));
        ret = read(fd1, read_buf, 2);
        if (-1 == ret){
            perror("read failure");
            _exit(-1);
        } else {
			printf("fd1：[%s].\n", read_buf);
		}
        sleep(1);
        memset(read_buf, 0, sizeof(read_buf));
        ret = read(fd2, read_buf, 2);
        if (-1 == ret){
            perror("read failure");
            _exit(-1);
        } else {
			printf("fd2：[%s].\n", read_buf);
		}
        i--;
    }


    #if 0
    //Read content alone    分别读取内容
    //read file content
    while (i)
    {
        memset(read_buf, 0, sizeof(read_buf));
        ret = read(fd1, read_buf, 2);
        if (-1 == ret){
            perror("read failure");
            _exit(-1);
        } else {
			printf("fd1：[%s].\n", read_buf);
		}
        sleep(1);
        memset(read_buf, 0, sizeof(read_buf));
        ret = read(fd2, read_buf, 2);
        if (-1 == ret){
            perror("read failure");
            _exit(-1);
        } else {
			printf("fd2：[%s].\n", read_buf);
		}
        i--;
    }
    #endif
    close(fd1);
	close(fd2);

    _exit(0);
}



/*
3.1.9.多次打开同一文件与O_APPEND


3.1.9.1、重复打开同一文件读取


(1)一个进程中两次打开同一个文件，然后分别读取，看结果会怎么样


(2)结果无非2种情况：一种是fd1和fd2分别读，第二种是接续读。

经过实验验证，证明了结果是fd1和fd2分别读。


(3)分别读说明：我们使用open两次打开同一个文件时，

fd1和fd2所对应的文件指针是不同的2个独立的指针。

文件指针是包含在动态文件的文件管理表中的，

所以可以看出linux系统的进程中不同fd对应的是不同的独立的文件管理表。

===============================

3.1.9.2、重复打开同一文件写入

(1)一个进程中2个打开同一个文件，得到fd1和fd2.然后看是分别写还是接续写？


(2)正常情况下我们有时候需要分别写，有时候又需要接续写，所以这两种本身是没有好坏之分的。

关键看用户需求

(3)默认情况下应该是：分别写（实验验证过的）

===========================================

3.1.9.3、加O_APPEND解决覆盖问题

(1)有时候我们希望接续写而不是分别写？办法就是在open时加O_APPEND标志即可

3.1.9.4、O_APPEND的实现原理和其原子操作性说明
(1)O_APPEND为什么能够将分别写改为接续写？关键的核心的东西是文件指针。

分别写的内部原理就是2个fd拥有不同的文件指针，并且彼此只考虑自己的位移。

但是O_APPEND标志可以让write和read函数内部多做一件事情，

就是移动自己的文件指针的同时也去把别人的文件指针同时移动。

（也就是说即使加了O_APPEND，fd1和fd2还是各自拥有一个独立的文件指针，

但是这两个文件指针关联起来了，一个动了会通知另一个跟着动）

=================================

(2)O_APPEND对文件指针的影响，对文件的读写是原子的。


(3)原子操作的含义是：整个操作一旦开始是不会被打断的，

必须直到操作结束其他代码才能得以调度运行，这就叫原子操作。

每种操作系统中都有一些机制来实现原子操作，以保证那些需要原子操作的任务可以运行。





3.1.10.文件共享的实现方式

3.1.10.1、什么是文件共享

(1)文件共享就是同一个文件

（同一个文件指的是同一个inode，同一个pathname）

被多个独立的读写体（几乎可以理解为多个文件描述符）

去同时（一个打开尚未关闭的同时另一个去操作）操作。

======================
(2)文件共享的意义有很多：

譬如我们可以通过文件共享来实现多线程同时操作同一个大文件，

以减少文件读写时间，提升效率。

========================================

3.1.10.2、文件共享的3种实现方式

(1)文件共享的核心就是怎么弄出来多个文件描述符指向同一个文件。

(2)常见的有3种文件共享的情况：

第一种是同一个进程中多次使用open打开同一个文件，

第二种是在不同进程中去分别使用open打开同一个文件

（这时候因为两个fd在不同的进程中，所以两个fd的数字可以相同也可以不同）


第三种情况是后面要学的，linux系统提供了dup和dup2两个API来让进程复制文件描述符。
==================================
分析一下第二种情况：


如果两个独立进程各自打开了同一文件

我们假定第一个进程使该文件在文件描述符3上打开，

而另一个进程则使此文件在文件描述符4上打开。

打开此文件的每一个进程都得到一个文件对象，

但对一个给定的文件只有一个v节点表项。

每个进程都有自己的文件对象的一个理由：

这种安排使每个进程都有它自己对该文件的当前位移量。

这种情况不会增加对应的打开文件引用计数，而会增加dentry的引用。（目录项）



（1） 在完成每一个write后，在文件表项中的当前文件位移量即增加所写的字节数。

如果这使当前文件位移量超过了当前文件长度，

则在i节点表象中的当前文件长度被设置为当前文件位移量(也就是该文件加长了)。


（2）   如果用O_APPEND标志打开一个文件，

则相应标志也被设置到文件表项（file对象）的文件状态标志中。

每次对这种具有填写标志的文件执行写操作时，

在文件表项中的当前文件位移量首先被设置为i节点表项中的文件长度。

这就使得每次写的数据都添加到文件的当前尾端处。

=====================================




(3)我们分析文件共享时的核心关注点在于：分别写/读还是接续写/读

3.1.10.3、再论文件描述符

(1)文件描述符的本质是一个数字，这个数字本质上是进程表中文件描述符表的一个表项，

进程通过文件描述符作为index去索引查表得到文件表指针，

再间接访问得到这个文件对应的文件表。

(2)文件描述符这个数字是open系统调用内部由操作系统自动分配的，

操作系统分配这个fd时也不是随意分配，也是遵照一定的规律的，我们现在就要研究这个规律。


(3)操作系统规定，fd从0开始依次增加。

fd也是有最大限制的，在linux的早期版本中（0.11）fd最大是20，

所以当时一个进程最多允许打开20个文件。linux中文件描述符表是个数组（不是链表），

所以这个文件描述符表其实就是一个数组，fd是index，文件表指针是value


(4)当我们去open时，内核会从文件描述符表中挑选一个最小的未被使用的数字给我们返回。


也就是说如果之前fd已经占满了0-9，那么我们下次open得到的一定是10.

（但是如果上一个fd得到的是9，下一个不一定是10，

这是因为可能前面更小的一个fd已经被close释放掉了）


(5)fd中0、1、2已经默认被系统占用了，因此用户进程得到的最小的fd就是3了。

(6)linux内核占用了0、1、2这三个fd是有用的，当我们运行一个程序得到一个进程时，

内部就默认已经打开了3个文件，

这三个文件对应的fd就是0、1、2。这三个文件分别叫stdin、stdout、stderr。

也就是标准输入、标准输出、标准错误。


(7)标准输入一般对应的是键盘（可以理解为：0这个fd对应的是键盘的设备文件），

标准输出一般是LCD显示器（可以理解为：1对应LCD的设备文件）


(8)printf函数其实就是默认输出到标准输出stdout上了。

stdio中还有一个函数叫fpirntf，这个函数就可以指定输出到哪个文件描述符中。

------------------
over

*/