const DEVICE_ID = "*********";
const LOGIN =  "**********";
const PASS = "********";

function ButtonManualControl(identifier) {
    require(['https://cdn.jsdelivr.net/particle-api-js/5/particle.min.js'], function (api) {
        identifier.style.background = 'greenyellow'; // Жёлтый цвет кнопки при первом нажатии
        var Particle = api;
        var particle = new Particle();

        particle.login({username: LOGIN, password: PASS}).then(
            function(data) {

                var token = data.body.access_token;
                console.log('login success 1', token);

                let manualState; // Состояние, включен ли мануальный режим или нет
                particle.getVariable({ deviceId: DEVICE_ID, name: 'manualMode', auth: token }).then(function(data) {
                    console.log('Device variable retrieved successfully:', data);
                    manualState = data.body.result;

                    // Включаем, если был в выключен ручной режим
                    if (manualState == "0"){
                        identifier.style.background = 'red';
                        let fnPr = particle.callFunction({ deviceId: DEVICE_ID, name: "userManualControl", argument: 'manualON', auth: token });
                        fnPr.then(
                            function(data) {

                                identifier.style.background = 'limegreen'; // зелёный цвет кнопки
                                console.log('Function called succesfully:', data);
                            }, function(err) {
                                console.log('An error occurred:', err);
                                alert(err);
                            });

                    // Выключаем, если был в ключен
                    } else if (manualState == "1"){
                        identifier.style.background = 'limegreen'; // красный, если выключен
                        let fnPr = particle.callFunction({ deviceId: DEVICE_ID, name: "userManualControl", argument: 'manualOFF', auth: token });
                        fnPr.then(
                            function(data) {
                                console.log('Function called succesfully:', data);
                                identifier.style.background = '#3b8ac7';
                            }, function(err) {
                                console.log('An error occurred:', err);
                                alert(err);
                            });
                    }
                }, function(err) {
                    console.log('An error occurred while getting attrs:', err);
                });
            },
            function (err) {
                console.log('Could not log in.', err);
            }
        );
    });
}


function ButtonUserSwapControl(identifier){
    require(['https://cdn.jsdelivr.net/particle-api-js/5/particle.min.js'], function (api) {
        identifier.style.background = 'greenyellow'; // Жёлтый цвет кнопки при первом нажатии
        var Particle = api;
        var particle = new Particle();

        particle.login({username: LOGIN, password: PASS}).then(
            function(data) {
                // Только тут и работать
                var token = data.body.access_token;
                console.log('login success 1', token);

                let TapState;
                particle.getVariable({ deviceId: DEVICE_ID, name: 'TapState', auth: token }).then(function(data) {
                    console.log('Device variable retrieved successfully:', data);
                    TapState = data.body.result;


                    // Открываем кран, если был открыт (0 - закрыт, 1 - открыт, ON - открыть, OFF - закрыть)
                    if (TapState == "0"){
                        identifier.style.background = 'red'; // красный, если выключен
                        let fnPr = particle.callFunction({ deviceId: DEVICE_ID, name: "userSwapControl", argument: 'ON', auth: token });
                        fnPr.then(
                            function(data) {

                                identifier.style.background = 'limegreen'; // зелёный цвет кнопки
                                console.log('Function called succesfully:', data);
                            }, function(err) {
                                console.log('An error occurred:', err);
                                alert(err);
                            });

                    // закрываем, если был открыт
                    } else if (TapState == "1"){
                        identifier.style.background = 'limegreen';
                        let fnPr = particle.callFunction({ deviceId: DEVICE_ID, name: "userSwapControl", argument: 'OFF', auth: token });
                        fnPr.then(
                            function(data) {
                                console.log('Function called succesfully:', data);

                                identifier.style.background = '#3b8ac7';
                            }, function(err) {
                                console.log('An error occurred:', err);
                                alert(err);
                            });
                    }
                }, function(err) {
                    console.log('An error occurred while getting attrs:', err);
                });

                },
            function (err) {
                console.log('Could not log in.', err);
            }
        );

    });

}


function ButtonAlarmControl(identifier) {
    identifier.style.background = 'greenyellow'; // Жёлтый цвет кнопки при первом нажатии

    require(['https://cdn.jsdelivr.net/particle-api-js/5/particle.min.js'], function (api) {
        var Particle = api;
        var particle = new Particle();

        particle.login({username: LOGIN, password: PASS}).then(
            function (data) {

                var token = data.body.access_token;
                console.log('login success 1', token);

                let TapState; // Состояние, ключен ли мануальный режим или нет
                particle.getVariable({deviceId: DEVICE_ID, name: 'alarmFlag', auth: token}).then(function (data) {
                    console.log('Device variable retrieved successfully:', data);
                    alarmFlag = data.body.result;


                    // Флаг тевоги на устройстве: 0 - всё ок. 1 - очень высокий уровень.
                    if (alarmFlag == "1") {
                        identifier.style.background = 'limegreen';
                        let fnPr = particle.callFunction({
                            deviceId: DEVICE_ID,
                            name: "userAlarmControl",
                            argument: 'OFF',
                            auth: token
                        });
                        fnPr.then(
                            function (data) {
                                console.log('Function called succesfully:', data);

                                identifier.style.background = 'red'; // Красный цвет кнопки
                            }, function (err) {
                                console.log('An error occurred:', err);
                                alert(err);
                            });
                    }
                }, function (err) {
                    console.log('An error occurred while getting attrs:', err);
                });
            },
            function (err) {
                console.log('Could not log in.', err);
            }
        );
    });
}

// Обновляет значнеия на странице, вызывается раз в 5 секунд
function updateValues() {

    require(['https://cdn.jsdelivr.net/particle-api-js/5/particle.min.js'], function (api) {
        var Particle = api;
        var particle = new Particle();

        particle.login({username: LOGIN, password: PASS}).then(
            function(data) {
                var token = data.body.access_token;
                console.log('login success for update', token);

                particle.getVariable({ deviceId: DEVICE_ID, name: 'currentLVL', auth: token }).then(function(data) {
                    console.log('Device variable retrieved successfully:', data);
                    document.getElementById("currentLVL").value = data.body.result;


                    // Считает проценты для строки состояний
                    let tempPercentLvL = + document.getElementById("currentLVL").value;
                    tempPercentLvL *= 100;
                    // document.getElementById("currentLVL").value = data.body.result; // Это на сотку умножать

                    tempPercentLvL /= document.getElementById("TankHeight").value;
                    document.getElementById("percentLVL").value = Math.round(tempPercentLvL) + '%';
                    bars.forEach((bar) => {
                        bar.style.width = Math.round(tempPercentLvL) + '%';
                    })
                }, function(err) {
                    console.log('An error occurred while getting attrs:', err);
                });


                particle.getVariable({ deviceId: DEVICE_ID, name: 'TapState', auth: token }).then(function(data) {
                    console.log('Device variable retrieved successfully:', data);
                    let element =  document.getElementById("TapState");

                    if (data.body.result == '1'){
                        element.value = "ОТКРЫТ";
                        element.style.background = 'yellowgreen';



                    } else if (data.body.result == '0'){
                        element.value = "ЗАКРЫТ";
                        element.style.background = 'red';

                    } else {
                        console.log('Value of TapState is unresolved');
                    }
                }, function(err) {
                    console.log('An error occurred while getting attrs:', err);
                });



                particle.getVariable({ deviceId: DEVICE_ID, name: 'alarmFlag', auth: token }).then(function(data) {
                    let element = document.getElementById("alarmFlag");
                    element.style.background = 'cornflowerblue';

                    console.log('Device variable retrieved successfully:', data);
                    if (data.body.result == '1'){
                        element.value = "ТРЕВОГА";
                        element.style.background = 'red';
                    } else {
                        element.value = "Всё в норме";
                        element.style.background = 'yellowgreen';
                    }
                }, function(err) {
                    console.log('An error occurred while getting attrs:', err);
                });


                particle.getVariable({ deviceId: DEVICE_ID, name: 'manualMode', auth: token }).then(function(data) {
                    let element = document.getElementById("manualMode");
                    element.style.background = 'cornflowerblue';

                    console.log('Device variable retrieved successfully:', data);
                    if (data.body.result == "1"){
                        element.value = "Ручной режим";
                        element.style.background = 'yellowgreen';
                    } else {
                        element.value = "Авто режим";
                    }

                }, function(err) {
                    console.log('An error occurred while getting attrs:', err);
                });



                particle.getVariable({ deviceId: DEVICE_ID, name: 'TankHeight', auth: token }).then(function(data) {
                    console.log('Device variable retrieved successfully:', data);
                    document.getElementById("TankHeight").value = data.body.result;

                }, function(err) {
                    console.log('An error occurred while getting attrs:', err);
                });


            },
            function (err) {
                console.log('Could not log in.', err);
            }
        );

    });

}


// Включает или выключает обновление данных с устройства
function ButtonCloseUpdate(element) {
    if (element.style.value == 'Включить обновление данных'){
        valIntervalUpd = setInterval( updateValues, 30000);
        element.style.background = '#3b8ac7';
        element.style.value = 'Выключить обновление данных';
    } else {
        clearInterval(valIntervalUpd);
        element.style.background = 'red';
        element.style.value = 'Включить обновление данных';
    }
}
