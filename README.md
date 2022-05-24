# NetSoakTest Project 

## Overview
NetSoakTest is an O3DE sample project used to stress test the AzNetworking transport layer. It is a headless server application that presently runs two connections locally via a loopback mechanism in which each endpoint sends a variety of packets to the other.

## Download and Install

This repository uses Git LFS for storing large binary files.  You will need to create a Github personal access token to authenticate with the LFS service.


### Create a Git Personal Access Token

You will need your personal access token credentials to authenticate when you clone the repository.

[Create a personal access token with the 'repo' scope.](https://docs.github.com/en/github/authenticating-to-github/creating-a-personal-access-token)


### (Recommended) Verify you have a credential manager installed to store your credentials 

Recent versions of Git install a credential manager to store your credentials so you don't have to put in the credentials for every request.  
It is highly recommended you check that you have a [credential manager installed and configured](https://github.com/microsoft/Git-Credential-Manager-Core)



### Clone the repository 

There are two options when installing a project

#### Option #1 (Recommended) - Project-centric approach to building a project 

This option lets you keep engine and project files in separate locations.

##### Windows
```shell
# clone the project into a folder outside your engine repository folder
> git clone https://github.com/o3de/o3de-netsoaktest.git
Cloning into 'o3de-netsoaktest'...

# register the engine
> C:/Path/To/o3de/scripts/o3de.bat register --this-engine 

# register the project 
> C:/Path/To/o3de/scripts/o3de.bat register -p C:/Path/To/o3de-netsoaktest

# example configure command
> cmake -S C:/Path/To/o3de-netsoaktest -b C:/Path/To/o3de-netsoaktest/build -G "Visual Studio 16 2019" -DLY_3RDPARTY_PATH="C:/3rdparty"

# example build command
> cmake --build C:/Path/To/o3de-netsoaktest/build --target Editor NetSoakTest.ServerLauncher --configure profile -- /m /nologo 
```

##### Linux
```shell
# clone the project into a folder outside your engine repository folder
> git clone https://github.com/o3de/o3de-netsoaktest.git
Cloning into 'o3de-netsoaktest'...

# register the engine
>/path/to/o3de/scripts/o3de.bat register --this-engine 

# register the project 
> /path/to/o3de/scripts/o3de.sh register -p /path/to/o3de-netsoaktest

# example configure command
> cmake -S /pathto/o3de-netsoaktest -b /path/to/o3de-netsoaktest/build -G "Ninja Multi-Config" -DLY_3RDPARTY_PATH="C:/3rdparty" -DCMAKE_C_COMPILER=clang-12 -DCMAKE_CXX_COMPILER=clang++-12 

# example build command
> cmake --build /path/to/o3de-netsoaktest/build --config profile --target Editor NetSoakTest.ServerLauncher
```


#### Option #2 - Engine-centric approach to building a project 

This option models the legacy folder layout where all projects were in the same folder as the engine.
If you use this approach you must also modify your engine's local git exclude file to ignore the project folder.

```shell
# clone the project in a folder named 'NetSoakTest' in your existing engine repository folder
> git clone https://github.com/o3de/o3de-netsoaktest.git C:/path/to/engine/NetSoakTest
Cloning into 'NetSoakTest'...

# modify the local engine git exclude file to ignore the project folder
> echo NetSoakTest > C:/Path/To/o3de/.git/info/exclude

# the project now exists inside the engine folder but your engine repository will ignore all files in the project folder

# example configure command
> cmake -S C:/Path/To/o3de -b C:/Path/To/o3de/build -G "Visual Studio 16 2019" -DLY_3RDPARTY_PATH="C:/3rdparty" -DLY_PROJECTS=NetSoakTest 

# example build command
> cmake --build C:/Path/To/o3de/build --target Editor NetSoakTest.ServerLauncher --configure profile -- /m /nologo 

```

If you have a Git credential helper configured, you should not be prompted for your credentials anymore.

## Running the Project

Run the netsoak ServerLauncher with the relevant options (see below). It is strongly recommended to use --rhi=null when launching NetSoakTest.ServerLauncher when running test. 

To pass command line values when launching the executable the format is ```--<command>=<value>```

``` 
NetSoakTest.ServerLauncher --soak_mode=loopback --rhi=null 
```

For experimentation with project, do not disable rhi so you can access the [debug console](https://www.o3de.org/docs/user-guide/appendix/cvars/debugging/#using-console-debug-views). The `DumpSoakStats` command can be used in the debug console to see point-in-time stats.

Note: All O3DE projects generate a GameLauncher and a ServerLauncher. NetSoakTest does not utilize its GameLauncher by design.

### Options


| Cvar | Description | Default |
|-------|------------|---------|
| soak_latencyms | Simulated connection quality latency in milliseconds | 0 | 
| soak_variancems | Simulated connection quality variance in milliseconds | 0 | 
| soak_losspercentage | Simulated connection quality packet drop rate | 0 |
| soak_serveraddr | The address for a client soak test to connect to, localhost by default (only used in Client Soak Mode) | 127.0.0.1 |
| soak_port | The port that this soak test will bind to for game traffic | 33450 |
| soak_protocol | Soak test protocol (TCP or UDP) | udp | 
| soak_mode | The operating mode for the soak test, options are loopback, client or host. `Loopback` has two connection within the application feed traffic to each other in a loop. `Client` expects to connect to a server hosted at soak_serveraddr. `Host` hosts a server for clients to connect to | Loopback | 
| DumpSoakStats | Dump snapshot of soak test networking stats to console | N/A|

Other networking features such as Compression or DTLS/TLS can be enabled/disabled in the same way they would be in a production environment. For example:

```
NetSoakTest.ServerLauncher --net_UdpUseEncryption=true
```

## License

For terms please see the LICENSE*.TXT file at the root of this distribution.

