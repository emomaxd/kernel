# nasm and qemu
sudo apt-get install nasm
sudo apt-get install qemu-system-x86
sudo apt-get install qemu-kvm

# GCC cross compiler for i686 systems (might take quite some time, prepare food)

sudo apt update
sudo apt install build-essential
sudo apt install bison
sudo apt install flex
sudo apt install libgmp3-dev
sudo apt install libmpc-dev
sudo apt install libmpfr-dev
sudo apt install texinfo

#cURL (needed to clone some required files)
sudo apt-get install curl


#Download GCC https://ftp.lip6.fr/pub/gcc/releases/
#Download binutils https://ftp.gnu.org/gnu/binutils/
#Extract binulits to src folder in home dir
export PREFIX="$HOME/opt/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

cd $HOME/src


#x.y.z will be your version
mkdir build-binutils
cd build-binutils
../binutils-x.y.z/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make
make install

cd $HOME/src

# The $PREFIX/bin dir _must_ be in the PATH. We did that above.
which -- $TARGET-as || echo $TARGET-as is notin the PATH

mkdir build-gcc
cd build-gcc
../gcc-x.y.z/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
make all-gcc
make all-target-libgcc
make install-gcc
make install-target-libgcc

# Check version
$HOME/opt/cross/bin/$TARGET-gcc --version