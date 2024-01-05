var host = location.host;

var ws = new WebSocket(`ws://${host}/`);
var user_id = null;
var results = {};

ws.onopen = () => {
  console.log("Connection Established");
};

ws.onclose = () => {
  console.log("Connection Closed");
};

const setResult = (value) => {
  resulthdr.innerText = `Count result: ${value}`;
};

const handleResponse = (response) => {
  if (response?.type === "id") {
    user_id = response.data;
  } else if (response?.type === "result") {
    result_request_id = response?.data?.request_id;
    result = response?.data?.result;

    // Request sent
    if (result_request_id in results) {
      setResult(result);
      delete results[result_request_id];

      btnsubmit.classList.remove("disabled");
    } else {
      results[result_request_id] = result;
    }
  }
};

ws.onmessage = (ev) => {
  const response = JSON.parse(ev.data);

  handleResponse(response);
};

ws.onerror = (ev) => {
  console.log(ev);
};

btnsubmit.onclick = async function () {
  if (!ws) {
    return;
  }

  let data = enteredtext.value;

  let response = await fetch(`http://${host}/api/count`, {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
    },
    body: JSON.stringify({
      id: user_id,
      data: data,
    }),
  });

  btnsubmit.classList.add("disabled");

  if (response.status != 200) {
    btnsubmit.classList.remove("disabled");
    return;
  }

  let body = await response.json();

  if (!body?.request_id) {
    btnsubmit.classList.remove("disabled");
    return;
  }

  let request_id = body?.request_id;

  // Already received result
  if (request_id in results) {
    btnsubmit.classList.remove("disabled");
    let result = results[request_id];

    setResult(result);

    delete results[request_id];
  } else {
    results[request_id] = null;
  }
};
