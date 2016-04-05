using UnityEngine;
using System.Collections;
using System.Linq;

public class Locations : MonoBehaviour {

	// Use this for initialization
    public UDPManager myUDPManager;
    public Camera myCamera;
    public GameObject mySphere;

	void Start () {

	}
	
	// Update is called once per frame
	void Update () {
        if (myUDPManager != null)
        {
            myCamera.transform.position = new Vector3((float)myUDPManager.tagdata.xGlass, (float)myUDPManager.tagdata.yGlass, (float)myUDPManager.tagdata.zGlass);


            mySphere.transform.position = new Vector3((float)myUDPManager.tagdata.xTag, (float)myUDPManager.tagdata.yTag, (float)myUDPManager.tagdata.zTag);
        }
	}
}
