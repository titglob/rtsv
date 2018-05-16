Le project rtsv permet d'inser dans votre projet certaines fonction d'api afin de faciler le debogge d'applications temps réel. Le résulat d'analyse peut provenir
de multiples sources et sont collectées par un serveur. Des fichiers waves analyzables avec gtwave ansi que des messages sequence chart sont générés dans des fichiers pdf. L'api est utilisable directement dans du code C mais aussi dans un bacth en ligne de commande.

Définition du besoin:
---------------------
Traquer les évenements internes et/ou externes sous forme de message sequence chart et/ou de waves, entre des processus non synchronizés (ex: de différents device O3, incluant le host). La vision vcd est l'unique outil permettant de visualiser simplement des changements de valeur de nombreuses variables, mais n'affiche pas la causalité des évenements contrairement aux msc, qui demeurent un outil de visualisation plus 'human frendly', mais dans laquelle la notion de temps (qu'il est possible de représenter), importe moins que la causalité des évenements.

L'ideal est d'utiliser des abstractions d'objects ou de placer dans les interfaces bas niveau ces triggers pour éviter d'envahir au maximum le code utilisateur.

L'idée générale est de fournir une api *unique*, sous forme textuelle ou d'api C, permettant de mettre en valeur des changements d'état principaux dans le code (octopus, leon, host) est de remonter les informations au host afin de les transcrire en msc, vcd, mais aussi pourquoi pas en dot, xhtml, ... pour décrire à un instant 't' un état d'allocation de ressources (ex: stream, channel, item..), ou décrire les chemins et transition du code en SDL.

Les méchanismes utilisés pour acheminer les informations de logs entre processus utilisent à priori des chemins indépendants. Attention à exclure les évenements
provenant des logs de ceux servant au systeme. Au niveau du host, les données arrivent des entités séparément ou de façon groupé (plus efficace).

On a besoin de plusieurs briques logicielles:
   - une librarie indépendante du sdk9 (et de PAL aussi pour pouvoir être gréffée dans l'octopus) qui servirait à :
      * convertir structure <-> buffer <-> command text en fonction de la commande.
      * gérer la notion de temps synchronisé (rt_time_t). Cette notion doit exprimer correctement la causalité et aussi donner être représentative du
        temps qui s'écoule, en choissant une base de temps commune (implique de connaitre les fréquences des processeurs).
      => int rt_msg_to_buf(buf, cmd, id1, id2, text)

   - le rtsv server, qui doit pouvoir:
      * lire sur un ou plusieurs fichiers texte ou binaire
      * générer une ou plusieurs sorties ou être pluggé sur d'autres outil (ex: msc_tracer).
      * faire le rapprochement des send_msg avec les recv_msg
      * trier par ordre croissant les messages queués
      * maintenir le current level de la simulation
      * permettre de grouper les objects en groupes ou sous groupes
      * support d'un mode de batch pour pouvoir avancer en pas a pas
      * utiliser des fichiers de configuration pour du formattage spécifique ex:
         * affiner l'ordre des process
         * choisir un formattage différents pour les objets (couleur, style)

   - une interface C pour instrumenter le code, avec une implémentation spécifique librarie spécifique
   - une implémentation spéficique pour chaque processeur pour
      * implémenter les fonctions d'api, ex:

          rt_inittask((t_object_id)tid, "HAL"):
               int n = rt_pack_msg(rt_buffer, RTSV_DEF_CMD_INITTASK, id, 0, obj_name)
               mbx->send(rt_buffer, n);

      * créer et gére le canal de communication vers le host

Pour les msc, on voudra faire apparaître graphiquement les informations suivantes:
  - la notion de groupe, qui servira a préfixer les noms des objects et à les trier par ordre alphabétique (ou via un fichier de conf) dans le diagramme
  - les processus, préfixés du nombre du groupe ex: master_hal, slave_hal
  - envoie et reception d'un message : on veut faire apparaitre les croisements de messages, les dates.
  - l'ajout d'actions
  - les changements d'états
  - l'ajout de commentaires
  - les timers (start, stop, timeout).
  - timelife : desactivé, activé, suspendu.
  - les objects de synchronization, style sémaphore, mutex, ...
  - detecter les pertes de messages ou encore l'apparition de messages.
  - optionnellement, on pourra fonctionner en mode untimed, cad que l'on s'interresse uniquement
    à l'ordre partiel des évenements, et non a la durée.
  - on a besoin d'un algorithme de coupure de page, scalable pour accepter des tailles de pages configurables.
  - les msc pouvant être volumineux, il est necessaire de pouvoir définir les plages sur lesquelles l'utilisateur souhaite observer les évenements.
    => directement corréler à l'algo de cesure de pages.

Pour les waves, on veut:

Choix d'implémentation:
-----------------------

L'unité de temps est le 'Systime', configurable. On suppose connu le temps de transmission d'un message d'un processeur à l'autre: transmitTime.
La notion de temps choisié doit garantir l'ordre partiel grace au MAX dans rt_sync(), mais tiens compte aussi du temps qui sécoule grace a vco().
L'avantage de cette grandeur est que l'on représente bien la causalité (a précé b => time(a) < time(b)), les écarts de temps, et que l'on peut choisir plus finement le *retard de processing*, cad la durée à partir de laquelle les messages non récus sont considérés comme perdus. La granularité pour le temps doit être configurable.

    rt_time_t rt_time():
       return vco() x RATIO + offTime

    rt_time_t rt_sync()(extTime):
       locTime = vco() x RATIO.
       ajustedTime = MAX(extTime + transmitTime, locTime + offTime) // afin de garantir la causalité.
       offTime = ajustedTime - locTime // reajust the systemTime 
       return ajustedTime

Les événements sont stackés dans une mailbox séparée, afin d'éviter de confondre les messages de debug des messages normaux. Le processeur poste des évenements dans sa boîte aux lettre locale, à destination du parent, qui vient lire les packets distants et les forward dans sa propre boite au lettre a destination d'un autre parent, jusqu'au host, qui écrit les data dans un seul et même fichier. Bien sure les boites aux lettres en amont sont supposéés être de taille plus conséquente. On veillera à optimiser les accès distant en faisant des lectures blocs, de plusieurs messages en même temps. L'avantage de cette methode est qu'à la fin des données peuvent se trouver dans un seul fichier.

On pourra créer en plus de la fonction Verbose une fonction synthethique pour chaque message. Cela permettra d'incorporer les msc en interne dans les fonctions sendMsg et waitMsg.

L'option ENABLE_RTSV permet d'activer le deboggage real-time dans le leon, le host ou l'octopus. La mailbox host est crée des le SdkApi_Open. La mailbox embaquée est crée au startup du leon. A chaque nouveau addCpu, on a une nouvelle mailbox supplémentaire à poller. On utilisera le com handler et l'irq habituelle.

Dans rtsv, au fur et a mesure, les messages qui arrivent au host sont lus, des objects sont créés, puis insérés dans une queue par ordre du plus ancien au plus vieux.
Si le plus vieux a une date "inférieure" à celle du plus récent d'au moins une durée configurable, alors le message est déqueué et l'on process pour ce message toutes les sorties actives.

Les objects générés peuvent être classés en groupes, nécessaires pour organiser et afficher plus clairement les données ou taches. On n'interdit pas les objets provenant de fichiers différents de faire partie d'un même groupe. l'id de chaque object reste unique. Au sein d'un groupe, les objects sont triés par ordre d'arrivé. Les groupes sont eux même des objets, donc possèdent un id unique. Graphiquement parlant, l'affichage des instances de msc ou des symbols vcd seront regroupés. Pour les msc comme pour les vcd, déclarer à postérieurie une instance pose problème car il faut connaitre la hierachie des le début. Ainsi, pour les vcd, le fichier de définition sera réellement écrit à la fin (sauf mode fifo), et pour les msc, à chaque nouvelle page l'ordre par groupe est rétablit mais sur une même page. Que se passe t-il si un object est supprimé. Pire si un autre est crée avec un autre type?




