#ifndef MUSICBOX_H
#define MUSICBOX_H

#include "Assignment.h"

#include "chai3d.h"
#include <ctime>
#include <iomanip> // setprecision

class MusicBox : public Assignment
{
private:
    // Data recording
    std::string fileName;
    float then = glfwGetTime();
    std::vector<float> speedVector;

    // A 3D cursor for the haptic device
    cShapeSphere* m_cursor;
    cShapeSphere* m_guide;

    // Material properties used to render the color of the cursors
    cMaterialPtr m_matCursorButtonON;
    cMaterialPtr m_matCursorButtonOFF;

    constexpr static int NUM_CLIPS = 30;
    // An audio device to play sounds
    cAudioDevice* m_audioDevice;
    std::vector<std::unique_ptr<cAudioBuffer>> m_audioBuffer;
    std::vector<std::unique_ptr<cAudioSource>> m_audioSource;

    cAudioBuffer* m_scareAudioBuffer;
    cAudioSource* m_scareAudioSource;
    int m_nextClip = 0;
    float m_volume = 0.9;
    float m_pitch = 0;

    //OBJECTS
    cMultiMesh* clown;
    cMultiMesh* box;
    cMultiMesh* lid;
    cMultiMesh* crank;

    cCamera* m_camera;
    GLFWwindow* m_window;

    cMatrix3d crankRot;

    cLabel* m_debugLabel;

    // Modes
    bool m_hapticsOn, m_soundsOn;
    int m_mode = 0;

    // Rotation
    int m_angle;
    int m_prevAngle, m_prevFrameAngle;
    int m_destAngle;
    int m_min = 270;
    int m_max = 360;
    int m_counter, m_internalCounter;
    bool m_readyForLap = false;
    float m_rad = 0.02;

    // Scare
    double m_clownScaleY = 1;
    int m_scare = 0;
    int m_scare_goal = 18;
    float m_vibration_percent = 1;
    float m_clown_reset_size = 0.1;

    // Camera
    cVector3d cam_location_start = cVector3d(0.15,0.1,0.1);
    cVector3d cam_look_start = cVector3d(0.0,0.0,0.03);
    cVector3d cam_location_end = cVector3d(0.05, 0.0, 0.05);
    cVector3d cam_look_end = cVector3d(0.0,0.0,0.06);

    cVector3d cam_location = cam_location_start;
    cVector3d cam_look = cam_look_start;
    cVector3d cam_up = cVector3d(0.0, 0.0, 1.0);
    float cam_percent = 0;

    // Functions
    void playNextSound();
    void reset();
    void scare();
    void setFileName();
    void writeSpeedData();
    void changeMode(size_t);
    void loadObjects(cWorld*);
    cVector3d randomShakeYourBooty(float);
    cVector3d lerp(cVector3d start, cVector3d end, float percent);

public:
    virtual std::string getName() const { return "Music Box "+std::to_string(m_mode); }

    virtual void initialize(cWorld* world, cCamera* camera, GLFWwindow*);
	virtual void updateGraphics();
	virtual void updateHaptics(cGenericHapticDevice* hapticDevice, double timeStep, double totalTime);

    virtual void setInitialized(bool value);
};

void MusicBox::initialize(cWorld* world, cCamera* camera, GLFWwindow* window)
{
    m_window = window;
    glfwSetWindowTitle(m_window, getName().c_str());

    m_soundsOn = true;
    m_hapticsOn = true;
    m_camera = camera;

	//Change the background
    world->setBackgroundColor(0.0f, 0.0f, 0.0f);

    // Create a cursor with its radius set
	m_cursor = new cShapeSphere(0.01);
	// Add cursor to the world
    //world->addChild(m_cursor);

    loadObjects(world);

//        m_guide = new cShapeSphere(m_rad);
//        // Add cursor to the world
//        world->addChild(m_guide);

	// Here we define the material properties of the cursor when the
	// user button of the device end-effector is engaged (ON) or released (OFF)

    // A light orange material color
    m_matCursorButtonOFF = cMaterialPtr(new cMaterial());
    m_matCursorButtonOFF->m_ambient.set(0.5, 0.2, 0.0);
    m_matCursorButtonOFF->m_diffuse.set(1.0, 0.5, 0.0);
    m_matCursorButtonOFF->m_specular.set(1.0, 1.0, 1.0);

    // A blue material color
    // A light orange material color
    m_matCursorButtonON = cMaterialPtr(new cMaterial());
    m_matCursorButtonON->m_ambient.set(0.1, 0.1, 0.4);
    m_matCursorButtonON->m_diffuse.set(0.3, 0.3, 0.8);
    m_matCursorButtonON->m_specular.set(1.0, 1.0, 1.0);

    // Apply the 'off' material to the cursor
    m_cursor->m_material = m_matCursorButtonOFF;

    std::cout << "Creating audio device" << std::endl;
    // Create new audio device
    m_audioDevice = new cAudioDevice();
    camera->attachAudioDevice(m_audioDevice);

    for(int i = 0; i < NUM_CLIPS; ++i) {
        m_audioSource.emplace_back(new cAudioSource);

        std::cout << "Creating audioBuffer #" << i << std::endl;
        m_audioBuffer.emplace_back(new cAudioBuffer);
        // load a WAV file
        if(m_audioBuffer[i]->loadFromFile("../assets/sounds/"+std::to_string(i+1)+".wav")) {
            std::cout << "Successfully loaded audioBuffer for clip #" << i+1 << "!" << std::endl;
            m_audioSource[i]->setAudioBuffer(m_audioBuffer[i].get());
            m_audioSource[i]->setGain(m_volume);
        } else {
            std::cout << "Failed to load audioBuffer." << std::endl;
        }
    }

    m_scareAudioBuffer = new cAudioBuffer;
    m_scareAudioSource = new cAudioSource;
    if(m_scareAudioBuffer->loadFromFile("../assets/sounds/monster.wav")) {
        std::cout << "Successfully loaded audioBuffer for SCARE" << std::endl;
        m_scareAudioSource->setAudioBuffer(m_scareAudioBuffer);
        m_scareAudioSource->setGain(m_volume);
    } else {
        std::cout << "Failed to load SCARE audioBuffer" << std::endl;
    }

    //Debug text
    cFontPtr font2 = NEW_CFONTCALIBRI20();

    // Create a label used to show how debug output can be handled
    m_debugLabel = new cLabel(font2);

    // Labels need to be added to the camera instead of the world
    camera->m_frontLayer->addChild(m_debugLabel);

    reset();
    m_nextClip = 0; // Nice
    setFileName();
}

void MusicBox::updateGraphics()
{
    //Debug text
    std::stringstream ss;

    /*ss << " angle " << m_angle
       << "\n PrevAngle " << m_prevAngle
       << "\n DestAngle " << m_destAngle
       << "\n counter " << m_counter
       << "\n speed " << (m_angle - m_prevFrameAngle)
          << "\n icounter " << m_internalCounter
       << "\n diff " << (m_prevAngle - m_angle)
       << "\n ready " << m_readyForLap;*/



    m_debugLabel->setText(ss.str());

    // Position the label
    m_debugLabel->setLocalPos(30, 150, 0);

    if(m_counter == m_scare_goal && !m_scare) {
        scare();
    }

    if(m_clownScaleY < 1 && m_scare) {
        double c = 1.2;
        m_clownScaleY *= c;
        clown->scaleXYZ(1,1,c);
        clown->setLocalPos(0,0,clown->getLocalPos().z()+m_clownScaleY*0.011);
    }

    crankRot.setAxisAngleRotationDeg(cVector3d(0,1,0), -m_angle-45);
    crank->setLocalRot(crankRot);

    m_camera->set( cam_location,    // camera position (eye)
                  cam_look,    // look at position (target)
                  cam_up);   // direction of the (up) vector
}

void MusicBox::updateHaptics(cGenericHapticDevice* hapticDevice, double timeStep, double totalTime)
{
	//Read the current position of the haptic device
	cVector3d newPosition;
    cVector3d vel;
	hapticDevice->getPosition(newPosition);
    hapticDevice->getLinearVelocity(vel);

    // update global variable for graphic display update
    hapticDevicePosition = newPosition;

	// Update position and orientation of cursor
    cVector3d reaction(0, -newPosition.z(), newPosition.y());


	//Tip: Use 4_HapticWall as a reference

    //if(cAbs(newPosition.x) > 0){
    float x_limiter = -(newPosition.x()*1000);
    //}

	cVector3d force(0, 0, 0);
    float len = newPosition.length();
    newPosition.normalize();

    force = newPosition*(m_rad-len)*1000;
    force.x(x_limiter);

    cVector3d proxyPos;
    proxyPos.x(0);
    proxyPos.y(newPosition.y()*m_rad);
    proxyPos.z(newPosition.z()*m_rad);

    m_cursor->setLocalPos(proxyPos);



    m_angle = m_counter*360 + (atan2f(newPosition.y(),newPosition.z())+M_PI) * 180 / M_PI;

    if(m_readyForLap) {
        if(m_angle < 90+m_counter*360 && m_internalCounter == 4)  {
            //Event
            m_counter++;
            m_internalCounter = 0;
            m_min = 270 + m_counter*360;
            m_max = 360 + m_counter*360;
            m_readyForLap = false;
        }
    }

    if(m_angle > m_destAngle && m_angle < m_destAngle+90) {
        m_destAngle += 90;
        m_internalCounter++;
        force += cVector3d(120.0,0.0,0.0)*m_hapticsOn;
        m_prevAngle = m_angle - 45;
        if(m_soundsOn)
            playNextSound();
        if(!m_scare) {
            cam_percent += 1.0/(float)m_scare_goal/25;
            cam_location = lerp(cam_location_start, cam_location_end-cVector3d(0.3,0.2,0.0), cam_percent);
            cam_look = lerp(cam_look_start, cam_look_end, cam_percent);
        }

        float now = glfwGetTime();
        speedVector.push_back(now-then);
        then = now;
    }

    float speed = (m_angle - m_prevAngle)/45;
    m_prevFrameAngle = m_angle;


    // TODO: Velocity check
    if(m_angle > m_prevAngle)
        force += reaction*m_counter*500/m_scare_goal*m_hapticsOn;


    if(m_angle > m_min) {
        m_readyForLap = true;
    } else {
        m_readyForLap = false;
    }

    if(m_scare) {
        force += randomShakeYourBooty(100)*m_hapticsOn;
    }

	//Set a force to the haptic device
	hapticDevice->setForce(force);
}

void MusicBox::setInitialized(bool value) {
    if(!value) {
        delete m_audioDevice;
        m_audioBuffer.clear();
        m_audioSource.clear();
    }
    m_isInitialized = value;
}

cVector3d MusicBox::lerp(cVector3d start, cVector3d end, float percent)
{
     return (start + percent*(end - start));
}

void MusicBox::playNextSound() {
    float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX) * 2 - 1;
    m_audioSource[m_nextClip]->setPitch(m_pitch*r+1);
    m_pitch += 0.0003;
    m_audioSource[m_nextClip]->play();
    std::cout << "Playing sound " << m_nextClip+1 << std::endl;
    m_nextClip = (m_nextClip+1)%(NUM_CLIPS);
}

void MusicBox::reset() {
    m_min = 270;
    m_max = 360;
    m_counter = m_destAngle = m_prevAngle = 0;
    m_internalCounter = 0;
    m_readyForLap = false;

    clown->setLocalPos(0,0,-0.02);
    clown->scaleXYZ(1,1,m_clown_reset_size/m_clownScaleY);
    m_clownScaleY *= m_clown_reset_size/m_clownScaleY;

    m_scare = 0;
    m_vibration_percent = 1;
    m_nextClip = 0;
    m_pitch = 0;

    cam_percent = 0;
    cam_location = cam_location_start;
    cam_look = cam_look_start;
    cam_up = cVector3d(0.0, 0.0, 1.0);

    speedVector.clear();
    int min = 0;
    int max = NUM_CLIPS;
    m_nextClip = rand()%(max-min + 1) + min;
}

void MusicBox::scare() {
    m_scare = 1;

    cam_location = cam_location_end;
    cam_look = cam_look_end;

    m_min = 270;
    m_max = 360;
    m_counter = m_destAngle = m_prevAngle = 0;
    m_internalCounter = 0;
    m_readyForLap = false;

    if(m_soundsOn)
        m_scareAudioSource->play();

    writeSpeedData();
}

void MusicBox::setFileName() {
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];

    time (&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer,sizeof(buffer),"%d-%m-%Y %I:%M:%S",timeinfo);
    std::string str(buffer);

    fileName = str;
}

void MusicBox::writeSpeedData() {
    // Create .csv file
    std::ofstream datafile;
    datafile.open("data/"+fileName+".csv", std::ios::app);
    std::string mode;
    switch(m_mode) {
        case 0:
            mode = "All";
            break;
        case 1:
            mode = "Haptics";
            break;
        case 2:
            mode = "Sound";
            break;
    }

    // Save speed data to the .csv file
    datafile << mode;
    for(int i = 1; i < speedVector.size(); ++i) {
        datafile << "," << std::setprecision(2) << speedVector[i];
    }
    datafile << "\n";
    datafile.close();

    speedVector.clear();
}

void MusicBox::changeMode(size_t modeId) {
    /*
     * 0 All
     * 1 Haptics only
     * 2 Sound only
     */

    reset();

    m_mode = modeId;

    glfwSetWindowTitle(m_window, getName().c_str());

    switch(modeId) {
        case 0:
            m_soundsOn = true;
            m_hapticsOn = true;
            break;
        case 1:
            m_soundsOn = false;
            m_hapticsOn = true;
            break;
        case 2:
            m_soundsOn = true;
            m_hapticsOn = false;
            break;
    }
}

cVector3d MusicBox::randomShakeYourBooty(float strength) {
    float r_x = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    float r_y = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    float r_z = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    float rx = (r_x*2)-1;
    float ry = (r_y*2)-1;
    float rz = (r_z*2)-1;

    if(m_vibration_percent > 0) {
        m_vibration_percent -= 0.001;
    } else {
        m_vibration_percent = 0;
    };

    return cVector3d(rx,ry,rz)*strength*m_vibration_percent;
}

void MusicBox::loadObjects(cWorld * world) {
    clown = new cMultiMesh();
    box = new cMultiMesh();
    lid = new cMultiMesh();
    crank = new cMultiMesh();

    std::vector<cMultiMesh*> objects = {clown, box, lid, crank};
    std::vector<std::string> names = {"clown_small", "box", "lid", "crank"};

    world->addChild(clown);
    world->addChild(box);
    world->addChild(lid);
    world->addChild(crank);

    for(size_t i = 0; i < objects.size(); ++i) {
        bool fileload;
        fileload = objects[i]->loadFromFile("../assets/obj/"+names[i]+".obj");
        if (!fileload)
        {
            std::cout << "Error - 3D Model failed to load correctly." << std::endl;
            close(-1);
        }
    }

    crank->m_material->m_specular.set(1.0, 1.0, 1.0);

    lid->setLocalPos(0,0,0.024);
    crank->setLocalPos(0,0.025,0);
}
#endif

