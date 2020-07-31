document.getElementById("sm").addEventListener("click", function(e){
    var us = document.getElementById("us").value;
    var pw = document.getElementById("pw").value;
    if(us == 'admin' && pw == 'admin'){
        var url = "info.html";
        window.location = url;
        e.preventDefault();
    }else{
        var url = "404.html";
        window.location = url;
        e.preventDefault();
    }
});