//
// SurgeEngine Plugin
// Copyright (C) 2018  Alexandre Martins <alemartf(at)gmail(dot)com>
// http://opensurge2d.org
//
//             uuuuuuuuuuuuuuuuuuuu                                    
//           u" uuuuuuuuuuuuuuuuuu "u                                  
//         u" u####################u "u 
//       u" u########################u "u                              
//     u" u############################u "u                            
//   u" u################################u "u                          
// u" u####################################u "u                        
// # ######################################## #                        
// # ######################################## #                        
// # ###" ... "#...  ...#" ... "###  ... "### #                        
// # ###u `"#######  ###  #####  ##  ###  ### #                        
// # ######uu "####  ###  #####  ##  """ u### #                        
// # ###""###  ####  ###u "###" u##  ######## #                        
// # ####....,#####..#####....,####..######## #                        
// # ######################################## #                        
// "u "####################################" u"                        
//   "u "################################" u"                          
//     "u "############################" u"                            
//       "u "########################" u"                              
//         "u "####################" u"                                
//           "u """""""""""""""""" u"                                  
//             """"""""""""""""""""             
// 
//        DO NOT MODIFY THE CODE BELOW!!!
//     unless you know what you are doing ;)
//

@Plugin
object "SurgeEngine"
{
    actorFactory = spawn("ActorFactory");
    inputFactory = spawn("InputFactory");
    sensorFactory = spawn("SensorFactory");
    levelManager = spawn("LevelManager");
    camera = spawn("Camera");
    mouse = spawn("Mouse");

    fun get_Actor()
    {
        return actorFactory;
    }

    fun get_Sensor()
    {
        return sensorFactory;
    }

    fun get_Input()
    {
        return inputFactory;
    }

    fun get_LevelManager()
    {
        return levelManager;
    }

    fun get_Level()
    {
        return levelManager.currentLevel;
    }

    fun get_Camera()
    {
        return camera;
    }

    fun get_Mouse()
    {
        return mouse;
    }
}

object "ActorFactory"
{
    fun call(spriteName)
    {
        actor = caller.spawn("Actor");
        actor.__sprite = spriteName;
        return actor;
    }
}

object "SensorFactory"
{
    obstacleMap = spawn("ObstacleMap");

    fun call(x, y, w, h)
    {
        if(Math.min(w, h) == 1) {
            sensor = caller.spawn("Sensor");
            sensor.__init(x, y, x + w - 1, y + h - 1, obstacleMap);
            return sensor;
        }
        else {
            message = "Invalid sensor dimensions for ";
            Application.crash(message + caller.__name);
            return null;
        }
    }
}

object "InputFactory"
{
    fun call(inputMap)
    {
        input = caller.spawn("Input");
        input.__init(inputMap);
        return input;
    }
}