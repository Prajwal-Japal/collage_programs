public class Exceptionmain {
    public static void main(String[] args) {
        int a, d;
        try {
            d = 0;
            a = 42 / d;
            System.out.println("This will not be printed due to exception.");
        } catch (ArithmeticException e) {
            System.out.println("Error: Division by zero is not allowed.");
        } finally {
            System.out.println("After the try-catch block.");
        }
    }
}
