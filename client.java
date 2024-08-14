import java.io.*;
import java.net.*;

public class Client {
    private static final String SERVER_ADDRESS = "localhost";
    private static final int SERVER_PORT = 12345;

    public static void main(String[] args) {
        try (Socket socket = new Socket(SERVER_ADDRESS, SERVER_PORT);
             BufferedReader userInput = new BufferedReader(new InputStreamReader(System.in));
             PrintWriter out = new PrintWriter(socket.getOutputStream(), true);
             BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()))) {

            String request;
            while (true) {
                // Display the menu
                System.out.println("Select an operation:");
                System.out.println("1: Add");
                System.out.println("2: Subtract");
                System.out.println("3: Multiply");
                System.out.println("4: Divide");
                System.out.println("5: Square Root");
                System.out.println("6: Power");
                System.out.println("0: Exit");
                System.out.print("Enter your choice: ");

                request = userInput.readLine();
                if ("0".equals(request)) {
                    break;
                }

                System.out.print("Enter operands: ");
                String operands = userInput.readLine();

                // Prepare the request string
                String requestMessage = request + " " + operands;
                out.println(requestMessage);

                String response = in.readLine();
                System.out.println("Server response: " + response);
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}