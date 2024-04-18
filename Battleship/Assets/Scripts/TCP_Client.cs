using UnityEngine;
using System;
using System.Net.Sockets;
using System.Text;
using System.Collections.Generic;
using UnityEngine.UIElements;

public class TCP_Client : MonoBehaviour
{
    public string ipAddress = "127.0.0.1"; // Change this to the IP address you want to connect to
    public int port = 5000; // Change this to the port you want to connect to 

    private TcpClient client;
    private NetworkStream stream;
    private byte[] receiveBuffer = new byte[1024]; // Adjust buffer size as needed

    public string nameInput;
    public string passwordInput;

    public List<string> positions;

    private bool selectAction;

    private void Start()
    {
        GameManager gameManager = GameObject.FindObjectOfType<GameManager>();
        positions = gameManager.shipsPositions;

    }

    public void onLoginClick()
    {
        selectAction = false;
    }

    public void onRegisterClick()
    {
        selectAction = true;
    }


    public void ConnectToServer()
    {
        try
        {
            client = new TcpClient(ipAddress, port);
            stream = client.GetStream();
            Debug.Log("Connected to server.");

            // Start asynchronous reading
            stream.BeginRead(receiveBuffer, 0, receiveBuffer.Length, ReceiveCallback, null);
        }
        catch (SocketException ex)
        {
            Debug.LogError($"SocketException: {ex.SocketErrorCode}");
            Debug.LogError($"Error connecting to server: {ex.Message}");
        }
        catch (Exception e)
        {
            Debug.LogError("Error connecting to server: " + e.Message);
        }
    }

    public void GetNameInputString(string name) {
        nameInput = name;
    } 

    public void GetPasswordInputString(string password)
    {
        passwordInput = password;
    }


    public void ReceiveCallback(IAsyncResult result)
    {
        try
        {
            int bytesRead = stream.EndRead(result);
            if (bytesRead <= 0)
            {
                Debug.Log("Disconnected from server.");
                return;
            }

            string message = Encoding.ASCII.GetString(receiveBuffer, 0, bytesRead);
            Debug.Log("Received message from server: " + message);

            if (message.Contains("register") || message.Contains("login"))
            {
                String nombre = nameInput;
                String contraseña = passwordInput;
                if (selectAction == true)
                {
                    byte[] registerMessage = ConstructMessage(new byte[] { 0b00001001 }, nombre, contraseña);
                    stream.Write(registerMessage, 0, registerMessage.Length);
                    Debug.Log("Registration details sent to server.");


                }
                else if (selectAction == false) 
                {
                    byte[] registerMessage = ConstructMessage(new byte[] { 0b00001000 }, nombre, contraseña);
                    stream.Write(registerMessage, 0, registerMessage.Length);
                    Debug.Log("Registration details sent to server.");

                }
                else
                {
                    Debug.Log("No funco");
                }

                // if (message == "login")
                // {
                //     // Construct login message
                //     byte[] loginMessage = ConstructMessage(new byte[] { 0b00001000 }, "SomeUsername", "SomeAuthKey");
                //     stream.Write(loginMessage, 0, loginMessage.Length);
                // }
                // else
                // {
                //     Debug.LogError("Invalid choice.");
                // }
            }

            // Continue listening for more messages
            stream.BeginRead(receiveBuffer, 0, receiveBuffer.Length, ReceiveCallback, null);
        }
        catch (Exception e)
        {
            Debug.LogError("Error receiving message: " + e.Message);
        }
    }

    public string CheckTileGuesses(String guess) {
        if (positions.Count != 0)
        {
            if (positions.Contains(guess))
            {
                positions.Remove(guess);
                return("ATINASTE UN BARCO EN LA POSICION " + guess);
            }
            else
            {
                return ("Disparo fallido");
            }
        }
        else
        {
            return ("Perdi");
        }
    }




   
    private byte[] ConstructMessage(byte[] action, string username, string authKey)
    {
        byte[] messageBytes = new byte[29]; // Total size: 1 (action) + 24 (username) + 4 (authKey)

        // Action
        Array.Copy(action, messageBytes, 1);

        // Username
        byte[] usernameBytes = Encoding.ASCII.GetBytes(username.PadRight(24));
        Array.Copy(usernameBytes, 0, messageBytes, 1, usernameBytes.Length);

        // AuthKey
        byte[] authKeyBytes = Encoding.ASCII.GetBytes(authKey.PadRight(4));
        Array.Copy(authKeyBytes, 0, messageBytes, 25, authKeyBytes.Length);

        return messageBytes;
    }




    private void OnDestroy()
    {
        if (client != null)
            client.Close();
    }
}