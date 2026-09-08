import java.util.*;

interface SmartDevice {
    void turnOn();
    void turnOff();
}

class SmartFan implements SmartDevice {
    public void turnOn() {
        System.out.println("Smart Fan is turned on.");
    }
    public void turnOff() {
        System.out.println("Smart Fan is turned off.");
    }
}

class SmartLight implements SmartDevice {
    public void turnOn() {
        System.out.println("Smart Light is turned on.");
    }
    public void turnOff() {
        System.out.println("Smart Light is turned off.");
    }
}

class SmartAC implements SmartDevice {
    public void turnOn() {
        System.out.println("Smart AC is turned on.");
    }
    public void turnOff() {
        System.out.println("Smart AC is turned off.");
    }
}

class Main{
    public static void main(String[] args) {
        SmartDevice fan = new SmartFan();
        fan.turnOn();
        fan.turnOff();

        SmartDevice light = new SmartLight();
        light.turnOn();
        light.turnOff();

        SmartDevice ac = new SmartAC();
        ac.turnOn();
        ac.turnOff();
    }
}
        

        