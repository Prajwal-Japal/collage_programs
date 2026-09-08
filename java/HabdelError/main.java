import java.util.Random;
class Handleerror{
    public static void main(String[] args) {
        int a=0, b=0, c=0;
        Random r = new Random();
        for (int i = 0; i < 10; i++) {
            try {
                b = r.nextInt();
                c = r.nextInt();
                a = 12345 / (b+c);
            } catch (ArithmeticException e) {
                System.out.println("Error: Division by zero or invalid operation. b=");

            }
            System.out.println("a: " + a);
        }
    }
}
    