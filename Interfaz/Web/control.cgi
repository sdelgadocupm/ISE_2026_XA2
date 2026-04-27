t <html><head><title>Control</title>
t <script language=JavaScript>
t function confirmDisable(f){
t   if(!confirm('¿Desactivar la alarma remotamente?')) return;
t   f.submit();
t }
t </script></head>
i pg_header.inc
t <div class="card">
t  <h2>Control remoto</h2>
t  <p>Desde aqui puedes desactivar la alarma en caso de falso positivo o intervencion remota autorizada.</p>
t  <form action="index.htm" method="post" name="ctrl">
t   <input type=hidden value="ctrl" name=pg>
t   <input type=hidden value="disable_alarm" name=cmd>
t   <input class="btn danger" type=button value="DESACTIVAR ALARMA" onclick="confirmDisable(this.form)">
t  </form>
t  <p class="hint">Recomendado: proteger con usuario/clave (realm/admin) en produccion.</p>
t </div>
i pg_footer.inc
. End of script must be closed with period.

