#自定义变量
#变量赋值
OBJ=$(patsubst %.c,%.o,$(wildcard ./*.c))
TARGET=server

#LDFLAGS=-L./src_a -L./src_so
#LIBS=-lAMydiv -lSoMyadd
LIBS=-ljson-c
#使用$(TARGET)，必须要加$符号
$(TARGET):$(OBJ)
	$(CC) $^ $(LDFLAGS) $(LIBS)  -o $@

%.o:%.c
	$(CC) -c $^ -o $@

.PHONY: clean

clean:
	@$(RM) $(OBJ) $(TARGET)

show:
	@echo $(RM)
	@echo $(CC)
	@echo $(CXX)
	@echo $(wildcard ./*.c)
	@echo $(patsubst %.c,%.o,$(wildcard ./*.c))