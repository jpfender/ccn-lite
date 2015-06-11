# Prerequisites

CCN-lite requires OpenSSL. You can use the following to install it:
* Ubuntu: `sudo apt-get install libssl-dev`
* OS X: `brew install openssl`

# Installation

1. Clone the repository:

```bash
git clone https://github.com/cn-uofbasel/ccn-lite
```

2. Set environment variable `$CCNL_HOME` and add the binary folder of CCN-lite to your `$PATH`:

```bash
export CCNL_HOME="`pwd`/ccn-lite"
export PATH=$PATH:"$CCNL_HOME/bin"
```

3. Build CCN-lite using `make`:

```bash
cd $CCNL_HOME/src
make clean all
```