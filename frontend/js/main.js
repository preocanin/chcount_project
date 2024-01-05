var ws = new WebSocket("ws://localhost:3000/");
var user_id = null;
var request_id = null;

ws.onopen = () => {
  console.log("Connection Established");
};

ws.onclose = () => {
  console.log("Connection Closed");
};

ws.onmessage = (ev) => {
  console.log(ev);

  const response = JSON.parse(ev.data);

  if (response?.type === "id") {
    user_id = response.data;
  } else if (response?.type === "result") {
    result_request_id = response?.data?.request_id;
    result = response?.data?.result;

    btnsubmit.classList.remove("disabled");
    if (result_request_id === request_id) {
      resulthdr.innerText = `Count result: ${result}`;
    } else {
      console.error(`Unknown request result`);
    }
  }
};

ws.onerror = (ev) => {
  console.log(ev);
};

btnsubmit.onclick = async function () {
  if (!ws) {
    return;
  }

  let data = enteredtext.value;

  let response = await fetch("http://localhost:3000/api/count", {
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

  request_id = body?.request_id;
};
