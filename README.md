# kernelExample from Actividad M3.1: Linux "Kernel Modules" 

## Change to act directory

```
cd examples/act
```

## Create Kernel module

```
make -C /usr/src/$(uname -r) M=`pwd` modules
```

## Install kernel module

```
sudo insmod charDev_KernelModule
```

## Use code for testing module

```
gcc gpioTest.c -o run
./run
```

## For uninstalling module

```
sudo rmmod charDev_KernelModule
```