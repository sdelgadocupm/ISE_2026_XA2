t <html><head><title>Estado</title>
t <meta http-equiv="refresh" content="2"></head>
i pg_header.inc
t <div class="card">
t  <h2>Estado de la alarma</h2>
t  <div class="grid">
t   <div class="kpi"><div class="label">Modo</div><div class="value">c s 1 <span class="pill %s">%s</span></div></div>
t   <div class="kpi"><div class="label">Estado de energía</div><div class="value">c s 2 %s</div></div>
t   <div class="kpi"><div class="label">Consumo</div><div class="value">c s 3 %.0f mA</div></div>
t   <div class="kpi"><div class="label">Última actualización</div><div class="value">c s 4 %s</div></div>
t   <div class="kpi"><div class="label">Fecha actual</div><div class="value">c s 5 %s</div></div>
t   <div class="kpi"><div class="label">Hora actual</div><div class="value">c s 6 %s</div></div>
t  </div>
t </div>
t <div class="card">
t  <h3>Últimas mediciones</h3>
t  <div class="grid">
t   <div class="kpi"><div class="label">Temperatura</div><div class="value">c s 7 %.1f &deg;C</div></div>
t   <div class="kpi"><div class="label">eCO2</div><div class="value">c s 8 %d ppm</div></div>
t   <div class="kpi"><div class="label">TVOC</div><div class="value">c s 9 %d ppb</div></div>
t  </div>
t  <p class="hint">La calidad del aire puede ser IAQ o ppm equivalente, según vuestro sensor.</p>
t </div>
i pg_footer.inc
. End of script must be closed with period.