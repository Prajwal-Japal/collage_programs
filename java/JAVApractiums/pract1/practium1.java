class BankAccount {
    String accountNumber;
    String accountHolderName;
    String accountType;
    double Balance;

    BankAccount(String accno, String name, String type, double balance) {
        accountNumber = accno;
        accountHolderName = name;cd
        accountType = type;
        Balance = balance;
    }

    void deposit(double amount) {
        if (amount > 0) {
            Balance = Balance + amount;
            System.out.println("Deposited: " + amount);
        } else {
            System.out.println("Invalid deposit amount.");
        }
    }

    void withdraw(double amount) {
        if (amount > 0 && amount <= Balance) {
            Balance = Balance - amount;
            System.out.println("Withdrawn: " + amount);
        } else {
            System.out.println("Invalid withdrawal amount or insufficient balance.");
        }
    }

    void BankEnquiry() {
        System.out.println("Current Balance: " + Balance);
    }
}

class practium1 {
    public static void main(String[] args) {
        BankAccount account = new BankAccount(
            "123456",
            "John Doe",
            "Savings",
            1000.0
        );

        System.out.println("Account Number: " + account.accountNumber);
        System.out.println("Account Holder: " + account.accountHolderName);
        System.out.println("Account Type: " + account.accountType);

        account.deposit(500.0);
        account.withdraw(200.0);
        account.BankEnquiry();
    }
}