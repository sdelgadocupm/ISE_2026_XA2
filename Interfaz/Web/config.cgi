t <html><head><title>Configuracion</title>
t <script language=JavaScript>
t function confirmSave(f){
t   if(!confirm('¿Guardar configuracion?')) return;
t   f.submit();
t }
t </script></head>
i pg_header.inc
t <div class="card">
t  <h2>Configuracion</h2>
t  <p class="hint">Ajusta parametros del sistema. (Luego podeis persistir en EEPROM).</p>
t  <form action="index.htm" method="post" name="cfg">
t   <input type=hidden value="cfg" name=pg>
t   <table>
t    <tr><th>Parametro</th><th>Valor</th></tr>
t    <tr>
t     <td>Umbral de temperatura (alarma)</td>
t     <td>c k 1 <input type="text" name="th_temp" value="%.1f" size="6"> &deg;C</td>
t    </tr>
t    <tr>
t     <td>Umbral de eCO2</td>
t     <td>c k 2 <input type="text" name="th_co2" value="%d" size="6"> ppm</td>
t    </tr>
t    <tr>
t     <td>Umbral de TVOC</td>
t     <td>c k 3 <input type="text" name="th_tvoc" value="%d" size="6"> ppb</td>
t    </tr>
t   </table>
t   <p style="margin-top:12px">
t     <input class="btn" type=button value="Guardar" onclick="confirmSave(this.form)">
t     <input class="btn secondary" type=reset value="Deshacer">
t   </p>
t  </form>
t </div>
i pg_footer.inc
. End of script must be closed with period.