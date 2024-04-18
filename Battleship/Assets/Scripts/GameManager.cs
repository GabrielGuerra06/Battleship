using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text.RegularExpressions;
using UnityEngine;
using UnityEngine.SceneManagement;
using UnityEngine.UI;
using TMPro;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using UnityEditor;

public class GameManager : MonoBehaviour
{
    public string serverIP = "127.0.0.1"; // IP del servidor al que te quieres conectar
    public int serverPort = 8888; // Puerto del servidor al que te quieres conectar

    private TcpClient client;
    private NetworkStream stream;
    private byte[] buffer = new byte[1024];

    public List<string> positions;


    [Header("Ships")] public GameObject[] ships;
    public EnemyScript enemyScript; //quitar esto
    private ShipScript shipScript;
    private List<int[]> enemyShips;
    private int shipIndex = 0;
    public List<TileScript> allTileScripts;

    [Header("HUD")] public Button nextBtn;
    public Button rotateBtn;
    public Button replayBtn;
    public TMP_Text topText;
    public TMP_Text playerShipText;
    public TMP_Text enemyShipText;

    [Header("Objects")] public GameObject missilePrefab;
    public GameObject enemyMissilePrefab;
    public GameObject firePrefab;
    public GameObject woodDock;

    private bool setupComplete = false;
    private bool playerTurn = true;

    private List<GameObject> playerFires = new List<GameObject>();
    private List<GameObject> enemyFires = new List<GameObject>();

    private int enemyShipCount = 5;
    private int playerShipCount = 5;

    public List<string> player1Ships = new List<string>();

    public List<string> shipsPositions = new List<string>();


    // Start is called before the first frame update
    void Start()
    {
        shipScript = ships[shipIndex].GetComponent<ShipScript>();
        nextBtn.onClick.AddListener(() => NextShipClicked());
        rotateBtn.onClick.AddListener(() => RotateClicked());
        replayBtn.onClick.AddListener(() => ReplayClicked());
        enemyShips = enemyScript.PlaceEnemyShips();
    }


    private void ConnectToServer()
    {
        try
        {
            client = new TcpClient(serverIP, serverPort);
            stream = client.GetStream();

            Debug.Log("Conectado al servidor");

            stream.BeginRead(buffer, 0, buffer.Length, ReceiveData, null);
        }
        catch (Exception e)
        {
            Debug.LogError("Error al conectar al servidor: " + e.Message);
        }
    }

    private void ReceiveData(IAsyncResult ar)
    {
        try
        {
            int bytesRead = stream.EndRead(ar);
            if (bytesRead > 0)
            {
                string receivedData = Encoding.ASCII.GetString(buffer, 0, bytesRead);
                Debug.Log("Datos recibidos del servidor: " + receivedData);

                if (receivedData.Contains("Prueba"))
                {
                    SendData("A3");
                }
                else if (receivedData.Contains("Espera"))
                {
                    stream.BeginRead(buffer, 0, buffer.Length, ReceiveData, null);
                }
                else if (receivedData == "A3")
                {
                    SendData(CheckTileGuesses(receivedData));

                }
                else if(receivedData == "ATINASTE"){
                    
                }

                else if(receivedData == "FALLASTE"){
                    
                }

                else if(receivedData == "DERROTA"){
                    
                }



                stream.BeginRead(buffer, 0, buffer.Length, ReceiveData, null);
            }
            else
            {
                Debug.Log("El servidor ha cerrado la conexión");
            }
        }
        catch (Exception e)
        {
            Debug.LogError("Error al recibir datos del servidor: " + e.Message);
        }
    }

    public void SendData(string data)
    {
        try
        {
            byte[] byteData = Encoding.ASCII.GetBytes(data);
            stream.Write(byteData, 0, byteData.Length);
            Debug.Log("Datos enviados al servidor: " + data);
        }
        catch (Exception e)
        {
            Debug.LogError("Error al enviar datos al servidor: " + e.Message);
        }
    }

    private void NextShipClicked()
    {
        if (!shipScript.OnGameBoard())
        {
            shipScript.FlashColor(Color.red);
        }
        else
        {
            if (shipIndex <= ships.Length - 2)
            {
                shipIndex++;
                shipScript = ships[shipIndex].GetComponent<ShipScript>();
                shipScript.FlashColor(Color.yellow);
                String lastElement = player1Ships.Last();
                shipsPositions.Add(lastElement);
            }
            else
            {
                rotateBtn.gameObject.SetActive(false);
                nextBtn.gameObject.SetActive(false);
                woodDock.SetActive(false);
                topText.text = "Guess an enemy tile.";
                setupComplete = true;
                for (int i = 0; i < ships.Length; i++) ships[i].SetActive(false);
                String lastElement = player1Ships.Last();
                shipsPositions.Add(lastElement);
                for (int i = 0; i < shipsPositions.Count; i++)
                {
                    Debug.Log(shipsPositions[i]);
                }

                ConnectToServer();
            }
        }
    }

    public void TileClicked(GameObject tile)
    {
        if (setupComplete && playerTurn)
        {
            Vector3 tilePos = tile.transform.position;
            tilePos.y += 15;
            playerTurn = false;
            Instantiate(missilePrefab, tilePos, missilePrefab.transform.rotation);
        }
        else if (!setupComplete)
        {
            PlaceShip(tile);
            shipScript.SetClickedTile(tile);
        }
    }


    private void PlaceShip(GameObject tile)
    {
        shipScript = ships[shipIndex].GetComponent<ShipScript>();
        shipScript.ClearTileList();
        Vector3 newVec = shipScript.GetOffsetVec(tile.transform.position);
        ships[shipIndex].transform.localPosition = newVec;
        player1Ships.Add(tile.name);
    }


    void RotateClicked()
    {
        shipScript.RotateShip();
    }

    public string CheckTileGuesses(String guess) {
        if (positions.Count != 0)
        {
            if (positions.Contains(guess))
            {
                positions.Remove(guess);
                GameObject tile = GameObject.Find(guess);
                Vector3 vec = tile.transform.position;
                vec.y += 15;
                GameObject missile = Instantiate(enemyMissilePrefab, vec, enemyMissilePrefab.transform.rotation);
                return("ATINASTE");
            }
            else
            {
                return ("FALLASTE");
            }
        }
        else
        {
            return ("DERROTA");
        }
    }


    public void CheckHit(GameObject tile)
    {
        int tileNum = Int32.Parse(Regex.Match(tile.name, @"\d+").Value);
        int hitCount = 0;
        foreach (int[] tileNumArray in enemyShips)
        {
            if (tileNumArray.Contains(tileNum))
            {
                for (int i = 0; i < tileNumArray.Length; i++)
                {
                    if (tileNumArray[i] == tileNum)
                    {
                        tileNumArray[i] = -5;
                        hitCount++;
                    }
                    else if (tileNumArray[i] == -5)
                    {
                        hitCount++;
                    }
                }

                if (hitCount == tileNumArray.Length)
                {
                    enemyShipCount--;
                    topText.text = "SUNK!!!!!!";
                    enemyFires.Add(Instantiate(firePrefab, tile.transform.position, Quaternion.identity));
                    tile.GetComponent<TileScript>().SetTileColor(1, new Color32(68, 0, 0, 255));
                    tile.GetComponent<TileScript>().SwitchColors(1);
                }
                else
                {
                    topText.text = "HIT!!";
                    tile.GetComponent<TileScript>().SetTileColor(1, new Color32(255, 0, 0, 255));
                    tile.GetComponent<TileScript>().SwitchColors(1);
                }

                break;
            }
        }

        if (hitCount == 0)
        {
            tile.GetComponent<TileScript>().SetTileColor(1, new Color32(38, 57, 76, 255));
            tile.GetComponent<TileScript>().SwitchColors(1);
            topText.text = "Missed, there is no ship there.";
        }

        Invoke("EndPlayerTurn", 1.0f);
    }


    public void EnemyHitPlayer(Vector3 tile, int tileNum, GameObject hitObj)
    {
        enemyScript.MissileHit(tileNum);
        tile.y += 0.2f;
        playerFires.Add(Instantiate(firePrefab, tile, Quaternion.identity));
        if (hitObj.GetComponent<ShipScript>().HitCheckSank())
        {
            playerShipCount--;
            playerShipText.text = playerShipCount.ToString();
            enemyScript.SunkPlayer();
        }

        Invoke("EndEnemyTurn", 2.0f);
    }

    private void EndPlayerTurn()
    {
        for (int i = 0; i < ships.Length; i++) ships[i].SetActive(true);
        foreach (GameObject fire in playerFires) fire.SetActive(true);
        foreach (GameObject fire in enemyFires) fire.SetActive(false);
        enemyShipText.text = enemyShipCount.ToString();
        topText.text = "Enemy's turn";
        enemyScript.NPCTurn();
        ColorAllTiles(0);
        if (playerShipCount < 1) GameOver("ENEMY WINs!!!");
    }

    public void EndEnemyTurn()
    {
        for (int i = 0; i < ships.Length; i++) ships[i].SetActive(false);
        foreach (GameObject fire in playerFires) fire.SetActive(false);
        foreach (GameObject fire in enemyFires) fire.SetActive(true);
        playerShipText.text = playerShipCount.ToString();
        topText.text = "Select a tile";
        playerTurn = true;
        ColorAllTiles(1);
        if (enemyShipCount < 1) GameOver("YOU WIN!!");
    }

    private void ColorAllTiles(int colorIndex)
    {
        foreach (TileScript tileScript in allTileScripts)
        {
            tileScript.SwitchColors(colorIndex);
        }
    }

    void GameOver(string winner)
    {
        topText.text = "Game Over: " + winner;
        replayBtn.gameObject.SetActive(true);
        playerTurn = false;
    }


    void ReplayClicked()
    {
        SceneManager.LoadScene(SceneManager.GetActiveScene().name);
    }
}