class Car {
    int speed = 100;

    void display() {
        class Engine {
            void show() {
                System.out.println("Speed of engine is: " + speed);
            }
        }

        Engine engine = new Engine();
        engine.show();
    }
}

class NESTEDCLASS {
    public static void main(String[] args) {
        Car car = new Car();
        car.display();
    }
}