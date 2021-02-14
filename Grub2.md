This file describes how to make our kernel first in the boot order
in case you are planning to test on an evaluation machine that already
uses a kernel newer than 5.1.0. Other similar situations may arise
if you are using a non-default grub configuration. We assume an 
Ubuntu with grub2.0 for this tutorial.

## Check if you are running a kernel newer than 5.1.0

In the terminal input the following command:

```sh
$ uname -r
```

If this outputs a version > 5.1.0 then when our scripts automatically
load our kernel images they will not be first in the boot order.


## Make our kernel first in boot order.

Open the grub configuration file:

```sh
$ sudo gedit /etc/default/grub
```

In the opened file replace GRUB_DEFAULT=0 with 
GRUB_DEFAULT='Advanced options for Ubuntu>Ubuntu, with Linux 5.1.0'

Do a test update.
```sh
$ sudo update-grub2
```

## Optionally check if everything works

When running **run_artifact.sh** from remote machine (and LMBench is
running benchmarks) connect via ssh to the evaluation machine and type
**uname -r**. Should return 5.1.0.
