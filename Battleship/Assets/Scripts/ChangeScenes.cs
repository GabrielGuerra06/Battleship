using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;
public class ChangeScenes : MonoBehaviour
{



    public void Menu(int scene) 
    { 
        SceneManager.LoadScene(scene);

    }

    public void Login(int scene) 
    {
        SceneManager.LoadScene(scene);
    }

    public void Register(int scene) 
    {
        SceneManager.LoadScene(scene);

    }

    public void Games(int scene)
    {
        SceneManager.LoadScene(scene);

    }

    public void Play(int scene)
    {
        SceneManager.LoadScene(scene);
    }



}
