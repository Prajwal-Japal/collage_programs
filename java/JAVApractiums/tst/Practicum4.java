import java.util.Scanner;

class Payment {

    public void makePayment(double amount) {
        if (amount <= 0) {
            System.out.println("Invalid payment amount");
            return;
        }

        System.out.println("Payment Amount: Rs. " + amount);
    }

    // Method Overloading
    public void makePayment(double amount, String transactionId) {
        if (amount <= 0) {
            System.out.println("Invalid payment amount");
            return;
        }

        if (transactionId == null || transactionId.trim().isEmpty()) {
            System.out.println("Transaction ID cannot be empty");
            return;
        }

        if (!transactionId.matches("TXN\\d+")) {
            System.out.println("Invalid transaction ID format");
            return;
        }

        System.out.println("Payment Amount: Rs. " + amount);
        System.out.println("Transaction ID: " + transactionId);
    }
}

// Method Overriding
class CreditCardPayment extends Payment {

    @Override
    public void makePayment(double amount) {
        if (amount <= 0) {
            System.out.println("Invalid payment amount");
            return;
        }

        System.out.println("Credit Card Payment");
        System.out.println("Amount: Rs. " + amount);
    }
}

class UPIPayment extends Payment {

    @Override
    public void makePayment(double amount) {
        if (amount <= 0) {
            System.out.println("Invalid payment amount");
            return;
        }

        System.out.println("UPI Payment");
        System.out.println("Amount: Rs. " + amount);
    }
}

class NetBankingPayment extends Payment {

    @Override
    public void makePayment(double amount) {
        if (amount <= 0) {
            System.out.println("Invalid payment amount");
            return;
        }

        System.out.println("Net Banking Payment");
        System.out.println("Amount: Rs. " + amount);
    }
}

public class Practicum4 {

    public static void main(String[] args) {

        Scanner sc = new Scanner(System.in);

        System.out.println("1. Credit Card");
        System.out.println("2. UPI");
        System.out.println("3. Net Banking");
        System.out.println("4. Payment with Transaction ID");

        System.out.print("Enter choice: ");
        int choice = sc.nextInt();

        System.out.print("Enter amount: ");
        double amount = sc.nextDouble();

        Payment payment;

        switch (choice) {

            case 1:
                payment = new CreditCardPayment();
                payment.makePayment(amount);
                break;

            case 2:
                payment = new UPIPayment();
                payment.makePayment(amount);
                break;

            case 3:
                payment = new NetBankingPayment();
                payment.makePayment(amount);
                break;

            case 4:
                payment = new Payment();

                System.out.print("Enter Transaction ID: ");
                String transactionId = sc.next();

                payment.makePayment(amount, transactionId);
                break;

            default:
                System.out.println("Invalid choice");
        }

        sc.close();
    }
}