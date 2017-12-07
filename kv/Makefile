all:
	echo "Warning, this may take an hour or three on remote.  I suggest you use this time to grab a cup of coffee"
	sleep 10
	mkdir -p build
	-cd build && PATH=/home/kchiu/packages/linux-x86_64/bin:${PATH} LD_RUN_PATH=/home/kchiu/packages/linux-x86_64/libexec/gcc/x86_64-unknown-linux-gnu/7.2.0:/home/kchiu/packages/linux-x86_64/lib/gcc/x86_64-unknown-linux-gnu/7.2.0/plugin:/home/kchiu/packages/linux-x86_64/lib64:/home/kchiu/packages/linux-x86_64/lib32 cmake -DCMAKE_BUILD_TYPE=RELEASE ..
	cd build && make
