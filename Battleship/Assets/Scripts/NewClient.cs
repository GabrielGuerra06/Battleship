using System.Collections;
using System.Collections.Generic;
using UnityEngine;

using UnityEngine;
using System;
using System.Net.Sockets;
using System.Text;

public class NewClient : MonoBehaviour
{

    public string ipAddress = "127.0.0.1"; // Change this to the IP address you want to connect to
    public int port = 5000; // Change this to the port you want to connect to 

    private TcpClient client;
    private NetworkStream stream;
    private byte[] receiveBuffer = new byte[1024]; // Adjust buffer size as needed

    public string nameInput;
    public string passwordInput;

    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        
    }
}
