# 自定义变量
# 变量赋值
OBJ = $(patsubst %.c, %.o, $(wildcard ./*.c))
TARGET = server

LDFLAGS = -L./src_so
LIBS = -ljson-c -lsqlite3 -lSoMyTree

# 使用$(TARGET)，必须要加$符号
$(TARGET): $(OBJ)
	$(CC)  $^ $(LDFLAGS) $(LIBS) -o $@

%.o: %.c
	$(CC) -c $^ -o $@

.PHONY: clean

clean:
	@$(RM) $(OBJ) $(TARGET)

show:
	@echo $(RM)
	@echo $(CC)
	@echo $(wildcard ./*.c)
	@echo $(OBJ)
