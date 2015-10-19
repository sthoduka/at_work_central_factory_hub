RoCKIn@Work Referee Box
=======================

This is the repository of the referee box for the RoCKIn competition (http://rockinrobotchallenge.eu). This referee box is an adaption of the RoboCup Logistics League Sponsored by Festo (LLSF) referee box (http://www.robocup-logistics.org/refbox)


## Installation
The RoCKIn Central Factory Hub (CFH) can be installed on most linux distributions. However, some require dependencies being built from source. We support and test on Ubuntu LTS versions. The current recommended setup is Ubuntu 14.04, with Boost 1.54. For other setups, jump to the Alternative Setup section below

### Recommended Setup: Ubuntu 14.04, Boost 1.54

1. The CFH is based off LLSF refbox, and uses [clips](http://clipsrules.sourceforge.net/). Add [Tim's PPA](https://launchpad.net/~timn/+archive/ubuntu/clips) for clips:
      
        sudo add-apt-repository ppa:timn/clips
    (Note: This PPA is maintained by the developers of the LLSF referee box. And only works for Ubuntu 12.04, 12.10 and 14.04. For others, see Alternative Setup.)
    
2. Update apt:
        
        sudo apt-get update

3. Install the dependencies for both LLSFRB and CFH
        
        sudo apt-get install libmodbus-dev libclips-dev clips libclipsmm-dev \
                             protobuf-compiler libprotobuf-dev libprotoc-dev \
                             boost1.54-all-dev libmodbus-dev \
                             libglibmm-2.4-dev libgtkmm-3.0-dev \
                             libncursesw5-dev libyaml-cpp-dev libavahi-client-dev git \
                             libssl-dev libelf-dev mongodb-dev mongodb-clients \
                             mongodb libzmq3-dev
    (Note: Boost 1.54 is specified to avoid causing apt-get broken package problems with ROS. If you are using another version of Boost see Alternative Setup.)

4. Clone the CFH repo
        
        git clone https://github.com/rockin-robot-challenge/at_work_central_factory_hub.git

5. Enter the CFH repo
        
        cd at_work_central_factory_hub

6. Apply the following patch
    
        diff --git a/src/refbox/Makefile b/src/refbox/Makefile
        index aa05696..bd92a5c 100644
        --- a/src/refbox/Makefile
        +++ b/src/refbox/Makefile
        @@ -39,7 +39,7 @@ ifeq ($(HAVE_PROTOBUF)$(HAVE_LIBMODBUS)$(HAVE_CLIPS)$(HAVE_BOOST_LIBS),1111)
           CFLAGS  += $(CFLAGS_PROTOBUF) $(CFLAGS_LIBMODBUS) $(CFLAGS_CLIPS) $(CFLAGS_MONGODB) \
                     $(call boost-libs-cflags,$(REQ_BOOST_LIBS))
        -  LDFLAGS += $(LDFLAGS_PROTOBUF) $(LDFLAGS_LIBMODBUS) $(LDFLAGS_CLIPS) $(LDFLAGS_MONGODB) \
        +  LDFLAGS += $(LDFLAGS_PROTOBUF) $(LDFLAGS_LIBMODBUS) $(LDFLAGS_CLIPS) \
                     $(call boost-libs-ldflags,$(REQ_BOOST_LIBS))
        #MANPAGES_all =  $(MANDIR)/man1/llsf-refbox.1

7. Make the CFH project
        
        make

8. Go to Configuration Section before running the CFH.


### Alternative Setup
The CFH is known to work on several versions of Ubuntu, and ArchLinux. But the combinations of dependencies is very large, and they may cause conflicts. Users with alternative setups are assumed to be aware of their system dependencies.

1. To install the RoCKIn referee box, please read and install the prerequisites of the LLSF referee box which are described here:
   
    (Note: You MAY NOT be able to copy and paste from the following link. Ubuntu and Fedora instructions are provided. However, The instructions contain specific package versions, which are not available on all versions of Ubuntu. Search your package manager to find required versions.)
    
    https://trac.fawkesrobotics.org/wiki/LLSFRefBox/Install. 

2. Additionally, the following debian packages need to be installed (if available):
    
        sudo apt-get install libssl-dev libelf-dev mongodb-dev mongodb-clients mongodb libzmq3-dev

3. Older versions of Ubuntu are known to not have all of the above packages. The following may be installed from source if not available:
    - clips 6.30+ (http://sourceforge.net/projects/clipsmm/files/clips)
    - clipsmm 0.3.4+(http://sourceforge.net/projects/clipsmm/files/clipsmm)
    - zeromq 3+ (http://zeromq.org/) 
    - zeromq.hpp (https://github.com/zeromq/cppzmq/blob/master/zmq.hpp) 
            
            cd /usr/local/include/
            sudo wget https://raw.githubusercontent.com/zeromq/cppzmq/master/zmq.hpp
    - boost 1.48+ (http://www.boost.org/) It is STRONGLY recommended to install boost through your package manager, and to choose the version which does not conflict with your other dependencies (i.e. ROS)
  
4. Cloning the Git repository:
    
        git clone https://github.com/rockin-robot-challenge/at_work_central_factory_hub.git
        cd at_work_central_factory_hub
        git checkout rockin

4. Compiling the refbox:
    
        make


## Configuration

1. Configure the CFH with your editor
    
        editor ./cfg/config.yaml

    For testing the following changes are common:
    - Change the CFH IP address in section "shell"
        
            refbox-host: !ipv4 localhost
    - disable mongodb:
        
            enable: false
    - If testing on the same machine as the CFH is running, toggle the comments on "port" or "send-port" and "recv-port". To run the rockin-viewer and rockin-controller on the same machine, change in the section "public-peer", to run the robot on the same machine, the TeamName-peer secion must be edited.
    
        ```    
        #port: !udp-port 4444
        send-port: !udp-port 4444
        recv-port: !udp-port 4445
        ```


## Run the RoCKIn@Work Central Factory Hub

    ./bin/llsf-refbox
    

## FAQ

**Q:** I get the following error after executing the make command: 

    === Linking llsf-refbox ---
    /usr/bin/ld: /usr/lib/gcc/x86_64-linux-gnu/4.6/../../../../lib/libmongoclient.a(md5main.o): undefined reference to symbol 'sin@@GLIBC_2.2.5'
    /usr/bin/ld: note: 'sin@@GLIBC_2.2.5' is defined in DSO /usr/lib/gcc/x86_64-linux-gnu/4.6/../../../x86_64-linux-gnu/libm.so so try adding it to the linker command line
    /usr/lib/gcc/x86_64-linux-gnu/4.6/../../../x86_64-linux-gnu/libm.so: could not read symbols: Invalid operation
    collect2: ld returned 1 exit status
    
How can I solve the problem?

**A:** Please apply the following patch (Makefile Patch) and re ```make```:

    diff --git a/src/refbox/Makefile b/src/refbox/Makefile
    index aa05696..bd92a5c 100644
    --- a/src/refbox/Makefile
    +++ b/src/refbox/Makefile
    @@ -39,7 +39,7 @@ ifeq ($(HAVE_PROTOBUF)$(HAVE_LIBMODBUS)$(HAVE_CLIPS)$(HAVE_BOOST_LIBS),1111)
       CFLAGS  += $(CFLAGS_PROTOBUF) $(CFLAGS_LIBMODBUS) $(CFLAGS_CLIPS) $(CFLAGS_MONGODB) \
                 $(call boost-libs-cflags,$(REQ_BOOST_LIBS))
    -  LDFLAGS += $(LDFLAGS_PROTOBUF) $(LDFLAGS_LIBMODBUS) $(LDFLAGS_CLIPS) $(LDFLAGS_MONGODB) \
    +  LDFLAGS += $(LDFLAGS_PROTOBUF) $(LDFLAGS_LIBMODBUS) $(LDFLAGS_CLIPS) \
                 $(call boost-libs-ldflags,$(REQ_BOOST_LIBS))
    #MANPAGES_all =  $(MANDIR)/man1/llsf-refbox.1
 
 ------------------------------------

**Q:** I get an error complaining about duplicate sock exception
    
    Attempt to add global initializer failed, status: DuplicateKey throwSockExcepAttempt to add global initializer failed, status: DuplicateKey throwSockExcep...

**A:** Apply the Makefile Patch above, then run ```make```

**Q:** I get an error at execution time about a singleton class

    alex@LinuxDesktop../at_work_central_factory_hub$ ./bin/llsf-refbox 
    Mon Oct 19 02:48:31 Assertion: 10352:Security is a singleton class
    0x63e5a3 0x6380f3 0x636cab 0x587233 0x655a5d 0x7fc2655cfe35 0x587c05 
      ./bin/llsf-refbox(_ZN5mongo15printStackTraceERSo+0x23) [0x63e5a3]
      ./bin/llsf-refbox(_ZN5mongo11msgassertedEiPKc+0x93) [0x6380f3]
      ./bin/llsf-refbox() [0x636cab]
      ./bin/llsf-refbox() [0x587233]
      ./bin/llsf-refbox(__libc_csu_init+0x5d) [0x655a5d]
      /lib/x86_64-linux-gnu/libc.so.6(__libc_start_main+0x85) [0x7fc2655cfe35]
      ./bin/llsf-refbox() [0x587c05]
    terminate called after throwing an instance of 'mongo::MsgAssertionException'
      what():  Security is a singleton class
    Aborted (core dumped)

**A:** Apply the Makefile Patch above, then run ```make```

**Q:** I get the following error after executing the make command: 

    /usr/include/mongo/client/../util/net/../../db/../util/../db/mongomutex.h:235:9: error: call of overloaded ‘msgasserted(int, std::basic_string<char>)’ is ambiguous
    
How can I solve the problem?
    
**A:** Please apply the following patch: https://12286394519788571250.googlegroups.com/attach/8d1000635e6a8fa8/mongomutex.patch?part=0.1&view=1&vt=ANaJVrHyLNIISBTuORhFBABOAVPN-88t-nVX0FUuzvgxk8-w6O181B7fkE5fJsyydUHwq1vpbUzDvqvP3GFhuBFc8-EjaOnoLds8eox2JDGrk4FasLzQc_I

------------------------------------

