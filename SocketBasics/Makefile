.SUFFIXES: .java .class
.java.class:
	javac $*.java

CLASSES = client.java

default: classes

classes: $(CLASSES:.java=.class)

clean:
	$(RM) *.class