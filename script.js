var isapplied = false;

document.addEventListener('DOMContentLoaded', function () {
    const lock = document.getElementById('lock');
    const unlock = document.getElementById('unlock');

    //js for servo control lock
    lock.addEventListener('click', function () {

        //apply glow
        if(lock.classList.contains('active')){
            lock.classList.remove('active');
        }
        else{
            lock.classList.add('active');
        }
        
        // Send a request to the ESP8266 when the button is clicked
        fetch('https://bulldog-promoted-accurately.ngrok-free.app/lock', {
        method: "get",
        headers: new Headers({
            'Access-Control-Allow-Origin' : '*',
            'ngrok-skip-browser-warning': '69420',
        }),
        })
        .then(response => {
            if (!response.ok) {
                throw new Error('HTTP error! Status: ${response.status}');
            }
            return response.text();
        })
        .then(data => {
            console.log(data); // Log the response from the ESP8266
        })
        .catch(error => {
            console.error('Error:', error);
        });
    });

    //js for servo control unlock
    unlock.addEventListener('click', function () {

        //apply glow
        if(unlock.classList.contains('active')){
            unlock.classList.remove('active');
        }
        else{
            unlock.classList.add('active');
        }
        
        // Send a request to the ESP8266 when the button is clicked
        fetch('https://bulldog-promoted-accurately.ngrok-free.app/unlock', {
        method: "get",
        headers: new Headers({
            'Access-Control-Allow-Origin' : '*',
            'ngrok-skip-browser-warning': '69420',
        }),
        })
        .then(response => {
            if (!response.ok) {
                throw new Error('HTTP error! Status: ${response.status}');
            }
            return response.text();
        })
        .then(data => {
            console.log(data); // Log the response from the ESP8266
        })
        .catch(error => {
            console.error('Error:', error);
        });
    });
});