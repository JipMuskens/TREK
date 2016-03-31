using UnityEngine;
using System.Collections;
using UnityEngine.UI;

public class Distance : MonoBehaviour {

    public GameObject canera;
    public GameObject target;
    public UDPManager UDPmanager;
    public Text distancetxt;
    public Text targettxt;
    public Text searched;
    private float distance;

	// Use this for initialization
	void Start () {
	
	}
	
	// Update is called once per frame
	void Update () {
        distance = Vector3.Distance(canera.transform.position, target.transform.position);
        try
        {
            distancetxt.text = distance.ToString("F2") + "m";
            if (distance <= 5)
            {
                Color mycolor = new Color(distance/5, 1, distance/5);
                target.GetComponent<Renderer>().material.color = mycolor;
                target.GetComponent<Renderer>().material.SetColor("_EmissionColor", mycolor);
            }
            else
            {
                Color mycolor = new Color(1, 1, 1);
                target.GetComponent<Renderer>().material.color = mycolor;
                target.GetComponent<Renderer>().material.SetColor("_EmissionColor", mycolor);
            }
        }
        catch
        {
            distancetxt.text = "0m";
        }

        targettxt.text = UDPmanager.tagdata.IdTag;

        if(UDPmanager.playSound == "1")
        {
            searched.text = "true";
        }
        else
        {
            searched.text = "false";
        }
	}
}
