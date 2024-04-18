using UnityEngine;
using System.Net;
using System.Net.Sockets;
using TMPro;

public class ClientUDP : MonoBehaviour
{
    public string serverIP;
    public short serverPort;
    private UdpClient _udpClient;
    private IPEndPoint _remoteEndPoint;
    private string _logOutCode = "Logging out from Chat - Bye bye :)";
    public TMP_InputField messageBox;
    public GameObject chatBox;
    public GameObject textMessage;
    
    void Start()
    {
        _udpClient = new UdpClient();
        _remoteEndPoint = new IPEndPoint(IPAddress.Parse(serverIP), serverPort);
        Sent("Hello I'm a Player!");
    }

    void Update()
    {
        Receive();
    }

    private void OnApplicationQuit()
    {
        Sent(_logOutCode);
        _udpClient.Close();
    }

    public void Sent(string msg)
    {
        if (!string.IsNullOrWhiteSpace(messageBox.text))
        {
            byte[] data = System.Text.Encoding.ASCII.GetBytes(messageBox.text, 0, Mathf.Min(messageBox.text.Length, 1024));
            _udpClient.Send(data, data.Length, _remoteEndPoint);
            messageBox.text = "";
        }
        if (!string.IsNullOrWhiteSpace(msg))
        {
            byte[] data = System.Text.Encoding.ASCII.GetBytes(msg, 0, Mathf.Min(msg.Length, 1024));
            _udpClient.Send(data, data.Length, _remoteEndPoint);
            System.Text.Encoding.ASCII.GetBytes(msg);
        }
    }

    private void Receive()
    {
        if (_udpClient.Available > 0)
        {
            IPEndPoint remoteEndPoint = null;
            byte[] data = _udpClient.Receive(ref remoteEndPoint);
            string msg = System.Text.Encoding.ASCII.GetString(data, 0, Mathf.Min(data.Length, 1024));
            if (!string.IsNullOrWhiteSpace(msg))
            {
                GameObject message = textMessage;
                message.GetComponent<TextMeshProUGUI>().text = msg;
                Instantiate(message, chatBox.transform);
            }
        }
    }
}