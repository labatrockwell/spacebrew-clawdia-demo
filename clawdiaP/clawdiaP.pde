import spacebrew.*;
 
String server="127.0.0.1";//"sandbox.spacebrew.cc";
String name="Meet Clawdia";
String description ="This is a client that connects to ";
import processing.serial.*;
 
Serial myPort; // Create object from Serial class
boolean serial_connected = false;
 
Spacebrew sb;
int numClicks = 0;
int sec0;
 
boolean toSend = false;
 
void setup() {
frameRate(240);
size(400, 200);
 
sb = new Spacebrew( this );
 
// add each thing you publish and subscribe to
sb.addSubscribe( "rotate", "range" );
sb.addSubscribe( "swing", "range" );
sb.addSubscribe( "grab", "boolean" );
sb.addSubscribe( "grabWithPrecision", "range" );
 
// connect!
sb.connect(server, name, description );
 
connect_serial_by_index(12);
}
 
void draw() {
background( 255, 0, 0 );
fill(255);
textSize(30);
textAlign(CENTER);
text("beware of the CLAW!", width/2, height/2 );
}
 
void onRangeMessage( String name, int value ) {
println("got range message "+ name +" : "+ value);
if (name.indexOf("rotate") == 0) {
String message = "R" + value + "\n";
myPort.write(message);
println("Rotation message "+ message);
}
else if (name.indexOf("swing") == 0) {
String message = "S" + value + "\n";
myPort.write(message);
println("Swing message "+ message);
}
else if (name.indexOf("grabWithPrecision") == 0) {
String message = "G" + value + "A" + "\n";
myPort.write(message);
println("Grab message "+ message);
}
}
 
void onBooleanMessage( String name, boolean value ) {
println("got boolean message "+name +" : "+ value);
if (name.indexOf("grab") == 0) {
String message = "G" + (value ? 1 : 0) + "\n";
myPort.write(message);
println("Grab on/off message " + message);
}
}
 
public void connect_serial_by_index(int serial_device_num) {
String serial_list [] = Serial.list();
println(serial_list);
 
if (serial_device_num < serial_list.length) {
try {
myPort = new Serial(this, serial_list[serial_device_num], 9600);
delay(3000);
myPort.bufferUntil('\n');
myPort.write('\n');
serial_connected = true;
}
catch(Exception e) {
serial_connected = false;
}
}
 
println("\n** connection status **");
if (serial_connected) println("found serial: " + serial_list[serial_device_num]);
else println ("ERROR: serial NOT found");
}
 
void serialEvent(Serial myPort) {
String newMsg = "";
while (myPort.available () > 0) {
char new_byte = char(myPort.read());
newMsg = newMsg + new_byte;
}
print("[serialEvent] message received " + newMsg);
}
 
void mousePressed() {
String message = "R"+ int(random(0, 1023)) + "S" + int(random(0, 1023)) + "G" + int(random(0, 1)) + "\n";
myPort.write(message);
println("[keyPressed] sending random movement message: " + message);
}
