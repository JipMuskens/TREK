using UnityEngine;
using System.Collections;

public class Click : MonoBehaviour {

    private bool doWindow0 = false;
    void DoWindow0(int windowID)
    {
        if (GUI.Button(new Rect(10, 30, 80, 20), "Ok.")) { doWindow0 = false; };
    }
    void OnGUI()
    {
        //doWindow0 = GUI.Toggle(new Rect(10, 10, 100, 20), doWindow0, "Window 0");
        if (doWindow0)
            GUI.Window(0, new Rect(110, 10, 200, 60), DoWindow0, "You clicked!");
    }

    void Update()
    {
        if (Input.GetMouseButtonDown(0))
        {
            if(doWindow0 == false)
            {
                doWindow0 = true;
            }
        }
    }
}
