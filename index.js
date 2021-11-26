const Alexa = require('ask-sdk-core');
const AWS = require('aws-sdk');
const IoTData = new AWS.IotData({endpoint: 'aqg8dgtz1k884-ats.iot.us-east-2.amazonaws.com'});

let userName="";

let FeedFullParams = {
    topic: 'ucb/testIoT_catFeeder',
    payload: '{"action": "FULL"}',
    qos: 0
};

let FeedHalfParams = {
    topic: 'ucb/testIoT_catFeeder',
    payload: '{"action": "HALF"}',
    qos: 0
};

var paramsShadow = {
  thingName: "", 
};

var listOfUsers ={
    "Camila":"CatFeeder2",
    "Carolina":"MyThing"
};

const LaunchRequestHandler = {
  canHandle(handlerInput) {
    return Alexa.getRequestType(handlerInput.requestEnvelope) === 'LaunchRequest';
  },
  handle(handlerInput) {
    const speechText = 'Welcome to your feeder, what is your name?';

    return handlerInput.responseBuilder
      .speak(speechText)
      .reprompt(speechText)
      .getResponse();
  }
};

const CaptureNameIntentHandler = {
    canHandle(handlerInput) {
        return Alexa.getRequestType(handlerInput.requestEnvelope) === 'IntentRequest'
            
            && Alexa.getIntentName(handlerInput.requestEnvelope) === 'NameIntent';
    },
    handle(handlerInput) {
        let speechText="";
        userName = Alexa.getSlotValue(handlerInput.requestEnvelope, 'PersonName');
        if(listOfUsers[userName]!=undefined){
            FeedFullParams.topic= "ucb/testIoT_catFeeder/"+userName;
            FeedHalfParams.topic= "ucb/testIoT_catFeeder/"+userName;
            paramsShadow.thingName = listOfUsers[userName] ;   
            speechText = 'Welcome '+ userName+', if you want to feed yout cat please specify the quantity of food you want to give to your cat. You can also ask me if your cat is around.';
        }
        else{
            speechText="Sorry "+userName+" you don´t have a Cat Feeder"; 
        }
        return handlerInput.responseBuilder
            .speak(speechText)
            .reprompt(speechText)
            .getResponse();
    }
};

const CaptureFeedFullIntentHandler = {
    canHandle(handlerInput) {
        return Alexa.getRequestType(handlerInput.requestEnvelope) === 'IntentRequest'
            && Alexa.getIntentName(handlerInput.requestEnvelope) === 'IntentFeedCatFull';
    },
    handle(handlerInput) {
        IoTData.publish(FeedFullParams, function(err, data) {
            if (err) {
                console.log(err);
            }
            
        });
        
        const speechText = 'I´ll give your cat a full cup of food';

        return handlerInput.responseBuilder
            .speak(speechText)
            .reprompt(speechText)
            .getResponse();
    }
};

const CaptureFeedHalfIntentHandler = {
    canHandle(handlerInput) {
        return Alexa.getRequestType(handlerInput.requestEnvelope) === 'IntentRequest'
            && Alexa.getIntentName(handlerInput.requestEnvelope) === 'IntentFeedCatHalf';
    },
    handle(handlerInput) {
        IoTData.publish(FeedHalfParams, function(err, data) {
            if (err) {
                console.log(err);
            }
        });

        const speechText = 'I´ll give your cat half of a cup of food';

        return handlerInput.responseBuilder
            .speak(speechText)
            .reprompt(speechText)
            .getResponse();
    }
};

const CaptureIntentVerifyCatHandler = {
        
    canHandle(handlerInput) {
        return Alexa.getRequestType(handlerInput.requestEnvelope) === 'IntentRequest'
            && Alexa.getIntentName(handlerInput.requestEnvelope) === 'IntentVerifyCat';
    },
    handle(handlerInput) {
        IoTData.getThingShadow(paramsShadow, function(err, data) {
            if (err) {
              console.log(err, err.stack); // an error occurred
             }else{
               console.log(data);
               var speechText="";
               var jsonPayload = JSON.parse(data.payload);
               var  devState = jsonPayload.state.desired;
                var distanceInt =Number(JSON.stringify(devState.distance));
                if(distanceInt<20){
                   speechText = "Your cat may be Near";
                }
                else{
                  speechText = "Your cat may be Far";
                }
                speechText = speechText+". The distance is "+distanceInt;
                return handlerInput.responseBuilder
                    .speak(speechText)
                    .reprompt(speechText)
                    .getResponse();
               }
        });

        const speechText = 'I´ll measure the distance ';

        return handlerInput.responseBuilder
            .speak(speechText)
            .reprompt(speechText)
            .getResponse();
    }
};

const TurnOffIntentHandler = {
    canHandle(handlerInput) {
        return Alexa.getRequestType(handlerInput.requestEnvelope) === 'IntentRequest'
            && Alexa.getIntentName(handlerInput.requestEnvelope) === 'TurnOffIntent';
    },
    handle(handlerInput) {
        IoTData.publish(FeedHalfParams, function(err, data) {
            if (err) {
                console.log(err);
            }
        });

        const speechText = 'Apagado';

        return handlerInput.responseBuilder
            .speak(speechText)
            .reprompt(speechText)
            .getResponse();
    }
};

const HelpIntentHandler = {
  canHandle(handlerInput) {
    return Alexa.getRequestType(handlerInput.requestEnvelope) === 'IntentRequest'
      && Alexa.getIntentName(handlerInput.requestEnvelope) === 'AMAZON.HelpIntent';
  },
  handle(handlerInput) {
    const speechText = 'You have to tell me the quantity of food that you want to give to your cat: Half of a cup or a Full cup. Otherwise you can ask me to measure the distance.';

    return handlerInput.responseBuilder
      .speak(speechText)
      .reprompt(speechText)
      .withSimpleCard('You have to tell me the quantity of food that you want to give to your cat: Half of a cup or a Full cup', speechText)
      .getResponse();
  }
};

const CancelAndStopIntentHandler = {
  canHandle(handlerInput) {
    return Alexa.getRequestType(handlerInput.requestEnvelope) === 'IntentRequest'
      && (Alexa.getIntentName(handlerInput.requestEnvelope) === 'AMAZON.CancelIntent'
        || Alexa.getIntentName(handlerInput.requestEnvelope) === 'AMAZON.StopIntent');
  },
  handle(handlerInput) {
    const speechText = 'Good Bye! Now your cat is fed and happy!';

    return handlerInput.responseBuilder
      .speak(speechText)
      .withSimpleCard('Good Bye your! Now cat is fed and happy!', speechText)
      .withShouldEndSession(true)
      .getResponse();
  }
};

const ErrorHandler = {
  canHandle() {
    return true;
  },
  handle(handlerInput, error) {
    console.log(`Error handled: ${error.message}`);

    return handlerInput.responseBuilder
      .speak('Sorry, I don\'t understand your command. Please say it again.')
      .reprompt('Sorry, I don\'t understand your command. Please say it again.')
      .getResponse();
  }
};

const SessionEndedRequestHandler = {
  canHandle(handlerInput) {
    return Alexa.getRequestType(handlerInput.requestEnvelope) === 'SessionEndedRequest';
  },
  handle(handlerInput) {
    return handlerInput.responseBuilder.getResponse();
  }
};

exports.handler = Alexa.SkillBuilders.custom()
  .addRequestHandlers(
    LaunchRequestHandler,
    CaptureNameIntentHandler,
    CaptureFeedFullIntentHandler,
    CaptureFeedHalfIntentHandler,
    CaptureIntentVerifyCatHandler,
    TurnOffIntentHandler,
    HelpIntentHandler,
    CancelAndStopIntentHandler,
    SessionEndedRequestHandler)
  .addErrorHandlers(ErrorHandler)
  .lambda();