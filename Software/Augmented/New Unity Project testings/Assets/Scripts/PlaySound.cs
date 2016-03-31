using UnityEngine;
using System.Collections;

public class PlaySound : MonoBehaviour {

    private AudioSource myAudioSource;
    private bool alreadyplayed;
    public GameObject cameralocation;
    public UDPManager udpManager;

	// Use this for initialization
	void Start () {
        myAudioSource = GetComponent<AudioSource>();
        myAudioSource.loop = true;
        alreadyplayed = false;
	}
	
	// Update is called once per frame
	void Update () {
	    if(udpManager.playSound == "1")
        {
            if(myAudioSource.isPlaying == false)
            {
                myAudioSource.Play();
            }
        }
        else if(udpManager.playSound == "0")
        {
            myAudioSource.Stop();
        }
	}
}
