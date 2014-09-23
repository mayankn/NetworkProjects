.SUFFIXES: .java .class
.java.class:
	javac -classpath jsoup-1.7.2.jar $*.java

CLASSES = webcrawler.java

default: classes

classes: $(CLASSES:.java=.class)

clean:
	$(RM) *.class
