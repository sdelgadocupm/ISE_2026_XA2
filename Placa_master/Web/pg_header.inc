<body bgcolor="#f6f8fb" leftMargin=0 topMargin=10 marginwidth="0" marginheight="0">
<style>
  body { font-family: Verdana, Arial, sans-serif; margin:0; }
  .wrap { width: 900px; margin: 0 auto; }
  .top {
    background: #0b4a6f;
    color: #fff;
    border-radius: 10px;
    padding: 14px 18px;
    box-shadow: 0 8px 20px rgba(0,0,0,.12);
    margin-top: 10px;
  }
  .title { font-size: 20px; font-weight: 700; letter-spacing:.2px; }
  .subtitle { font-size: 12px; opacity:.85; margin-top: 2px; }
  .nav { margin-top: 10px; display:flex; gap:10px; flex-wrap:wrap; }
  .nav a {
    color:#e8f3ff; text-decoration:none;
    background: rgba(255,255,255,.10);
    padding: 8px 10px; border-radius: 8px;
    font-size: 13px;
  }
  .nav a:hover { background: rgba(255,255,255,.18); }
  .card {
    background:#fff; border-radius: 10px;
    padding: 14px 16px; margin: 14px 0;
    box-shadow: 0 6px 16px rgba(0,0,0,.08);
    border: 1px solid #e7edf3;
  }
  .grid { display:grid; grid-template-columns: 1fr 1fr; gap: 12px; }
  .kpi { display:flex; align-items:center; justify-content:space-between; padding: 10px 12px; border: 1px solid #eef2f6; border-radius: 10px; }
  .kpi .label { color:#567; font-size: 12px; }
  .kpi .value { font-weight:700; font-size: 16px; color:#123; }
  .pill { display:inline-block; padding: 4px 10px; border-radius: 999px; font-size:12px; font-weight:700; }
  .pill.ok { background:#e9f8ef; color:#1c7c3a; }
  .pill.warn { background:#fff5e6; color:#9a5a00; }
  .pill.alarm { background:#ffe8ea; color:#b42318; }
  table { width:100%; border-collapse: collapse; font-size: 13px; }
  th, td { padding: 10px 8px; border-bottom: 1px solid #eef2f6; text-align:left; }
  th { background: #f3f7fb; color:#234; font-weight:700; }
  .btn {
    background:#0b4a6f; color:#fff; border:0;
    padding:10px 12px; border-radius: 10px;
    cursor:pointer; font-weight:700;
  }
  .btn.danger { background:#b42318; }
  .btn.secondary { background:#2f6f91; }
  .hint { font-size: 12px; color:#678; }
  .footer { font-size: 11px; color:#678; padding: 10px 0 20px; }
</style>
<div class="wrap">
  <div class="top">
    <div class="title">Alarma de Incendios Inteligente</div>
    <div class="nav">
      <a href="/status.cgi">Estado</a>
      <a href="/history.cgi">Histórico</a>
      <a href="/control.cgi">Control</a>
      <a href="/config.cgi">Configuración</a>
    </div>
  </div>