function RPC(url, data, callback) {
  $.ajax({
    url: url,
    success: callback,
    dataType: "json",
    method: data ? "POST" : "GET",
    contentType: "text/json",
    data: data ? JSON.stringify(data) : null
  });
}

function makeButton(text, callback) {
  var but = document.createElement("input");
  but.type = "button";
  but.value = text;
  but.onclick = callback;
  return but;
}
