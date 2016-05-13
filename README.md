# An experiement in a class based object oriented language:

```
obj = new Object();
obj.name = "Ryan McCullagh";
obj.age = 25;

print("name: " + obj.name);
```

# A more complex example

```
class Container {
	
	func Container(obj) {
		self.properties = obj;
		self.getProperties().name = "Ryan McCullagh";
  }
	
	func setProperties(obj) {
		self.properties = obj;
	}
	
	func getProperties() {
		return self.properties;
	}
}

container = new Container(new Object());

props = container.getProperties();

print(props.name);
```
