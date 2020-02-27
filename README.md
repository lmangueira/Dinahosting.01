Se necesita desarrollar en C un entorno de comunicación entre varios servidores para poder administrarlos de manera unificada.
Para ello crearemos un programa ServidorDH.exe y otro ClienteDH.exe 
Deben ser comandos de consola preferiblemente. 
# ServidorDH.exe
- Una vez se ejecute en los servidores pasará a ocultarse la ventana y a funcionar continuamente. Como si de un servicio se tratase (no es necesario que responda a órdenes de servicio como iniciar, detener, etc.). 
- El programa escuchará conexiones en el puerto 4673. 
- Cada vez que haya una conexión se guardará en el fichero C:\ServidorDH\log.txt la fecha y hora de conexión, así como la dirección IP de origen de la misma y el comando ejecutado. 
- El programa ejecutará el comando que reciba, cuando finalice la ejecución devolverá un OK al cliente y volverá a escuchar conexiones. Si el comando recibido es reboot devolverá ERROR. 
# ClienteDH.exe
- Recibirá 4 parámetros: usuario, clave, servidor, comando (pidiéndolos por pantalla, directamente como argumentos, como tú quieras). 
- Comprobará en la base de datos gestiondh, tabla usuarios, si el usuario y contraseña son correctos, si lo son enviará el comando al servidor, de lo contrario finalizará. 
- El cliente esperará la respuesta del servidor, la mostrará por pantalla y finalizará. 