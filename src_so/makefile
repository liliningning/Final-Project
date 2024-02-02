#自定义变量
#变量赋值
OBJ=$(patsubst %.c,%.o,$(wildcard ./*.c))
TARGET=libSoMyTree.so

#使用$(TARGET)，必须要加$符号
$(TARGET):$(OBJ)
	$(CC) -g $^ -shared -fPIC -o $@
	cp $(TARGET) /usr/lib/
%.o:%.c
	$(CC) -g -c $^ -o $@

.PHONY: clean

clean:
	$(RM) $(OBJ) $(TARGET)
	$(RM) /usr/lib/$(TARGET)

show:
	$(RM) /usr/lib/$(TARGET)
	@echo $(wildcard ./*.c)
	@echo $(patsubst %.c,%.o,$(wildcard ./*.c))