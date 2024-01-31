
#变量的定义 
OBJ =$(patsubst %.c, %.o, $(wildcard ./*.c))
TARGET = client 



LIBS = -ljson-c -lsqlite3

$(TARGET):$(OBJ)
	$(CC) $^ $(LIBS)  -o $@


%.o:%.c
	$(CC) -c $^ -o $@

.PHONY: clean

clean:
	@$(RM) $(OBJ) $(TARGET)




