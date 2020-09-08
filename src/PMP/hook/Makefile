libhook.so: hook.o
	gcc -shared $< -o $@ -ldl

hook.o: hook.c
	gcc -pie -fpie -c -o $@ $<

clean:
	rm *.o *.so
